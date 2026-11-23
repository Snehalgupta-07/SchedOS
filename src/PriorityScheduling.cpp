#include "schedos/PriorityScheduling.h"
#include <algorithm>
#include <climits>

namespace SchedOS {

static int calculateEffectivePriority(const Process& p, int currentTime, int agingInterval) {
    if (agingInterval <= 0) return p.priority;
    int waitTime = std::max(0, currentTime - p.arrivalTime);
    int priorityBoost = waitTime / agingInterval;
    return std::max(0, p.priority - priorityBoost);
}

void PriorityScheduling::schedule(std::vector<Process>& processes) {
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
        int bestPriority = INT_MAX;

        for (int i = 0; i < n; ++i) {
            if (!completed[i] && processes[i].arrivalTime <= currentTime) {
                int effPrio = calculateEffectivePriority(processes[i], currentTime, agingInterval_);
                if (effPrio < bestPriority ||
                    (effPrio == bestPriority && bestIdx >= 0 &&
                     processes[i].arrivalTime < processes[bestIdx].arrivalTime)) {
                    bestPriority = effPrio;
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

            // Re-evaluate in case a higher-priority or newly aged job qualified
            bestIdx = -1;
            bestPriority = INT_MAX;
            for (int i = 0; i < n; ++i) {
                if (!completed[i] && processes[i].arrivalTime <= currentTime) {
                    int effPrio = calculateEffectivePriority(processes[i], currentTime, agingInterval_);
                    if (effPrio < bestPriority ||
                        (effPrio == bestPriority && bestIdx >= 0 &&
                         processes[i].arrivalTime < processes[bestIdx].arrivalTime)) {
                        bestPriority = effPrio;
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
