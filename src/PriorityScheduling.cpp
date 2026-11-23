#include "schedos/PriorityScheduling.h"
#include <algorithm>
#include <climits>

namespace SchedOS {

void PriorityScheduling::schedule(std::vector<Process>& processes) {
    if (processes.empty()) return;

    gantt_.clear();
    for (auto& p : processes) p.resetSchedulingState();
    sortByArrival(processes);

    int n = static_cast<int>(processes.size());
    std::vector<bool> completed(n, false);
    int currentTime = 0;
    int done = 0;

    while (done < n) {
        int bestIdx = -1;
        int bestPriority = INT_MAX;

        for (int i = 0; i < n; ++i) {
            if (!completed[i] && processes[i].arrivalTime <= currentTime) {
                if (processes[i].priority < bestPriority ||
                    (processes[i].priority == bestPriority && bestIdx >= 0 &&
                     processes[i].arrivalTime < processes[bestIdx].arrivalTime)) {
                    bestPriority = processes[i].priority;
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
            continue;
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
        ++done;
    }
}

} // namespace SchedOS
