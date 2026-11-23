#include "schedos/RoundRobin.h"
#include <algorithm>
#include <queue>
#include <climits>
#include <stdexcept>

namespace SchedOS {

RoundRobin::RoundRobin(int timeQuantum) : quantum_(timeQuantum) {
    if (quantum_ <= 0)
        throw std::invalid_argument("Time quantum must be positive");
}

void RoundRobin::setTimeQuantum(int q) {
    if (q <= 0)
        throw std::invalid_argument("Time quantum must be positive");
    quantum_ = q;
}

void RoundRobin::schedule(std::vector<Process>& processes) {
    if (processes.empty()) return;

    resetSchedulerState();
    for (auto& p : processes) p.resetSchedulingState();
    sortByArrival(processes);

    int n = static_cast<int>(processes.size());
    int currentTime = 0;
    int done = 0;
    int prevPid = -1;

    std::queue<int> readyQueue;
    std::vector<bool> inQueue(n, false);

    for (int i = 0; i < n; ++i) {
        if (processes[i].arrivalTime <= currentTime) {
            readyQueue.push(i);
            inQueue[i] = true;
        }
    }

    while (done < n) {
        if (readyQueue.empty()) {
            int nextArrival = INT_MAX;
            for (int i = 0; i < n; ++i) {
                if (processes[i].remainingTime > 0) {
                    nextArrival = std::min(nextArrival, processes[i].arrivalTime);
                }
            }
            gantt_.emplace_back(-1, currentTime, nextArrival);
            currentTime = nextArrival;
            prevPid = -1;

            for (int i = 0; i < n; ++i) {
                if (!inQueue[i] && processes[i].arrivalTime <= currentTime && processes[i].remainingTime > 0) {
                    readyQueue.push(i);
                    inQueue[i] = true;
                }
            }
            continue;
        }

        int idx = readyQueue.front();
        readyQueue.pop();

        auto& p = processes[idx];

        // Charge context switch if switching from a different process
        if (prevPid >= 0 && prevPid != p.pid && contextSwitchCost_ > 0) {
            gantt_.emplace_back(-2, currentTime, currentTime + contextSwitchCost_);
            currentTime += contextSwitchCost_;
            contextSwitchCount_++;
            totalContextSwitchOverhead_ += contextSwitchCost_;

            // Enqueue any processes that arrived DURING the context switch interval
            for (int i = 0; i < n; ++i) {
                if (!inQueue[i] && processes[i].arrivalTime <= currentTime && processes[i].remainingTime > 0) {
                    readyQueue.push(i);
                    inQueue[i] = true;
                }
            }
        }

        if (p.responseTime == -1) {
            p.responseTime = currentTime - p.arrivalTime;
        }

        int execTime = std::min(quantum_, p.remainingTime);
        int startTime = currentTime;
        currentTime += execTime;
        p.remainingTime -= execTime;

        p.executionIntervals.emplace_back(startTime, currentTime);
        gantt_.emplace_back(p.pid, startTime, currentTime);

        // Enqueue newly arrived processes during this quantum execution
        for (int i = 0; i < n; ++i) {
            if (!inQueue[i] && processes[i].arrivalTime <= currentTime && processes[i].remainingTime > 0) {
                readyQueue.push(i);
                inQueue[i] = true;
            }
        }

        if (p.remainingTime == 0) {
            p.completionTime = currentTime;
            p.turnaroundTime = p.completionTime - p.arrivalTime;
            p.waitingTime    = p.turnaroundTime - p.burstTime;
            ++done;
        } else {
            readyQueue.push(idx);
        }

        prevPid = p.pid;
    }
}

} // namespace SchedOS
