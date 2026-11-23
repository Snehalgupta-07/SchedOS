#include "schedos/SRTF.h"
#include <algorithm>
#include <climits>

namespace SchedOS {

void SRTF::schedule(std::vector<Process>& processes) {
    if (processes.empty()) return;

    gantt_.clear();
    for (auto& p : processes) p.resetSchedulingState();
    sortByArrival(processes);

    int n = static_cast<int>(processes.size());
    int currentTime = 0;
    int done = 0;
    int prevPid = -2;
    int ganttStart = 0;

    while (done < n) {
        int bestIdx = -1;
        int bestRemain = INT_MAX;

        for (int i = 0; i < n; ++i) {
            if (processes[i].remainingTime > 0 && processes[i].arrivalTime <= currentTime) {
                if (processes[i].remainingTime < bestRemain ||
                    (processes[i].remainingTime == bestRemain &&
                     processes[i].arrivalTime < processes[bestIdx].arrivalTime)) {
                    bestRemain = processes[i].remainingTime;
                    bestIdx = i;
                }
            }
        }

        if (bestIdx == -1) {
            int nextArrival = INT_MAX;
            for (int i = 0; i < n; ++i) {
                if (processes[i].remainingTime > 0)
                    nextArrival = std::min(nextArrival, processes[i].arrivalTime);
            }
            if (prevPid != -2 && prevPid != -1) {
                gantt_.emplace_back(prevPid, ganttStart, currentTime);
                for (auto& p : processes) {
                    if (p.pid == prevPid) {
                        p.executionIntervals.emplace_back(ganttStart, currentTime);
                        break;
                    }
                }
            }
            gantt_.emplace_back(-1, currentTime, nextArrival);
            prevPid = -1;
            ganttStart = nextArrival;
            currentTime = nextArrival;
            continue;
        }

        auto& p = processes[bestIdx];

        if (p.responseTime == -1) {
            p.responseTime = currentTime - p.arrivalTime;
        }

        if (p.pid != prevPid) {
            if (prevPid >= 0) {
                gantt_.emplace_back(prevPid, ganttStart, currentTime);
                for (auto& pp : processes) {
                    if (pp.pid == prevPid) {
                        pp.executionIntervals.emplace_back(ganttStart, currentTime);
                        break;
                    }
                }
            }
            ganttStart = currentTime;
            prevPid = p.pid;
        }

        p.remainingTime--;
        currentTime++;

        if (p.remainingTime == 0) {
            p.completionTime = currentTime;
            p.turnaroundTime = p.completionTime - p.arrivalTime;
            p.waitingTime    = p.turnaroundTime - p.burstTime;

            gantt_.emplace_back(p.pid, ganttStart, currentTime);
            p.executionIntervals.emplace_back(ganttStart, currentTime);

            prevPid = -2;
            ganttStart = currentTime;
            ++done;
        }
    }
}

} // namespace SchedOS
