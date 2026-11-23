#include "schedos/FCFS.h"
#include <algorithm>

namespace SchedOS {

void FCFS::schedule(std::vector<Process>& processes) {
    if (processes.empty()) return;

    resetSchedulerState();
    for (auto& p : processes) p.resetSchedulingState();
    sortByArrival(processes);

    int currentTime = 0;
    int prevPid = -1;

    for (auto& p : processes) {
        if (currentTime < p.arrivalTime) {
            gantt_.emplace_back(-1, currentTime, p.arrivalTime);
            currentTime = p.arrivalTime;
            prevPid = -1;
        }

        // Context switch overhead between different processes
        if (prevPid >= 0 && prevPid != p.pid && contextSwitchCost_ > 0) {
            gantt_.emplace_back(-2, currentTime, currentTime + contextSwitchCost_);
            currentTime += contextSwitchCost_;
            contextSwitchCount_++;
            totalContextSwitchOverhead_ += contextSwitchCost_;
        }

        p.responseTime = currentTime - p.arrivalTime;

        int startTime = currentTime;
        currentTime += p.burstTime;

        p.completionTime = currentTime;
        p.turnaroundTime = p.completionTime - p.arrivalTime;
        p.waitingTime    = p.turnaroundTime - p.burstTime;

        p.executionIntervals.emplace_back(startTime, currentTime);
        gantt_.emplace_back(p.pid, startTime, currentTime);

        prevPid = p.pid;
    }
}

} // namespace SchedOS
