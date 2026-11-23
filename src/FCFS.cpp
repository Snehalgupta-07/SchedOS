#include "schedos/FCFS.h"
#include <algorithm>

namespace SchedOS {

void FCFS::schedule(std::vector<Process>& processes) {
    if (processes.empty()) return;

    gantt_.clear();
    for (auto& p : processes) p.resetSchedulingState();
    sortByArrival(processes);

    int currentTime = 0;

    for (auto& p : processes) {
        if (currentTime < p.arrivalTime) {
            gantt_.emplace_back(-1, currentTime, p.arrivalTime);
            currentTime = p.arrivalTime;
        }

        p.responseTime = currentTime - p.arrivalTime;

        int startTime = currentTime;
        currentTime += p.burstTime;

        p.completionTime = currentTime;
        p.turnaroundTime = p.completionTime - p.arrivalTime;
        p.waitingTime    = p.turnaroundTime - p.burstTime;

        p.executionIntervals.emplace_back(startTime, currentTime);
        gantt_.emplace_back(p.pid, startTime, currentTime);
    }
}

} // namespace SchedOS
