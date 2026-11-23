#include "schedos/SRTF.h"
#include <algorithm>
#include <climits>

namespace SchedOS {

void SRTF::schedule(std::vector<Process>& processes) {
    if (processes.empty()) return;

    resetSchedulerState();
    for (auto& p : processes) p.resetSchedulingState();
    sortByArrival(processes);

    int n = static_cast<int>(processes.size());
    int currentTime = 0;
    int done = 0;
    int prevPid = -1;
    int currentRunPid = -1;
    int currentRunStart = 0;

    auto flushCurrentRun = [&](int endTime) {
        if (currentRunPid >= 0 && endTime > currentRunStart) {
            gantt_.emplace_back(currentRunPid, currentRunStart, endTime);
            for (auto& p : processes) {
                if (p.pid == currentRunPid) {
                    p.executionIntervals.emplace_back(currentRunStart, endTime);
                    break;
                }
            }
            currentRunPid = -1;
        }
    };

    while (done < n) {
        // Find process with shortest remaining time among arrived
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
            flushCurrentRun(currentTime);
            int nextArrival = INT_MAX;
            for (int i = 0; i < n; ++i) {
                if (processes[i].remainingTime > 0)
                    nextArrival = std::min(nextArrival, processes[i].arrivalTime);
            }
            gantt_.emplace_back(-1, currentTime, nextArrival);
            currentTime = nextArrival;
            prevPid = -1;
            continue;
        }

        auto& p = processes[bestIdx];

        // If switching from another process, charge context switch
        if (prevPid >= 0 && prevPid != p.pid && contextSwitchCost_ > 0) {
            flushCurrentRun(currentTime);
            gantt_.emplace_back(-2, currentTime, currentTime + contextSwitchCost_);
            currentTime += contextSwitchCost_;
            contextSwitchCount_++;
            totalContextSwitchOverhead_ += contextSwitchCost_;

            // Re-evaluate best process in case a shorter job arrived during the context switch
            bestIdx = -1;
            bestRemain = INT_MAX;
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
        }

        auto& runningProc = processes[bestIdx];

        if (runningProc.responseTime == -1) {
            runningProc.responseTime = currentTime - runningProc.arrivalTime;
        }

        if (runningProc.pid != currentRunPid) {
            flushCurrentRun(currentTime);
            currentRunPid = runningProc.pid;
            currentRunStart = currentTime;
        }

        // Execute for 1 unit of time
        runningProc.remainingTime--;
        currentTime++;
        prevPid = runningProc.pid;

        if (runningProc.remainingTime == 0) {
            flushCurrentRun(currentTime);
            runningProc.completionTime = currentTime;
            runningProc.turnaroundTime = runningProc.completionTime - runningProc.arrivalTime;
            runningProc.waitingTime    = runningProc.turnaroundTime - runningProc.burstTime;
            ++done;
        }
    }
    flushCurrentRun(currentTime);
}

} // namespace SchedOS
