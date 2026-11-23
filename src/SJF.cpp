#include "schedos/SJF.h"
#include <algorithm>
#include <climits>

namespace SchedOS {

void SJF::schedule(std::vector<Process>& processes) {
    if (processes.empty()) return;

    resetSchedulerState();
    for (auto& p : processes) p.resetSchedulingState();
    sortByArrival(processes);

    int n = static_cast<int>(processes.size());
    std::vector<bool> completed(n, false);
    int currentTime = 0;
    int done = 0;
    int prevPid = -1;

    while (done < n) {
        int bestIdx = -1;
        int bestBurst = INT_MAX;

        for (int i = 0; i < n; ++i) {
            if (!completed[i] && processes[i].arrivalTime <= currentTime) {
                if (processes[i].burstTime < bestBurst ||
                    (processes[i].burstTime == bestBurst && processes[i].arrivalTime < processes[bestIdx].arrivalTime)) {
                    bestBurst = processes[i].burstTime;
                    bestIdx = i;
                }
            }
        }

        if (bestIdx == -1) {
            int nextArrival = INT_MAX;
            for (int i = 0; i < n; ++i) {
                if (!completed[i])
                    nextArrival = std::min(nextArrival, processes[i].arrivalTime);
            }
            gantt_.emplace_back(-1, currentTime, nextArrival);
            currentTime = nextArrival;
            prevPid = -1;
            continue;
        }

        // If switching from another process, charge context switch
        if (prevPid >= 0 && prevPid != processes[bestIdx].pid && contextSwitchCost_ > 0) {
            gantt_.emplace_back(-2, currentTime, currentTime + contextSwitchCost_);
            currentTime += contextSwitchCost_;
            contextSwitchCount_++;
            totalContextSwitchOverhead_ += contextSwitchCost_;

            // Re-evaluate in case a shorter job arrived during the context switch
            bestIdx = -1;
            bestBurst = INT_MAX;
            for (int i = 0; i < n; ++i) {
                if (!completed[i] && processes[i].arrivalTime <= currentTime) {
                    if (processes[i].burstTime < bestBurst ||
                        (processes[i].burstTime == bestBurst && processes[i].arrivalTime < processes[bestIdx].arrivalTime)) {
                        bestBurst = processes[i].burstTime;
                        bestIdx = i;
                    }
                }
            }
        }

        auto& p = processes[bestIdx];

        p.responseTime = currentTime - p.arrivalTime;

        int startTime = currentTime;
        currentTime += p.burstTime;

        p.completionTime = currentTime;
        p.turnaroundTime = p.completionTime - p.arrivalTime;
        p.waitingTime    = p.turnaroundTime - p.burstTime;

        p.executionIntervals.emplace_back(startTime, currentTime);
        gantt_.emplace_back(p.pid, startTime, currentTime);

        completed[bestIdx] = true;
        prevPid = p.pid;
        ++done;
    }
}

} // namespace SchedOS
