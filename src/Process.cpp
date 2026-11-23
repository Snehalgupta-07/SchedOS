#include "schedos/Process.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace SchedOS {

Process::Process(int id, int arrival, int burst, int prio)
    : pid(id), arrivalTime(arrival), burstTime(burst), priority(prio),
      remainingTime(burst)
{
    validate();
}

void Process::validate() const {
    if (pid < 0)
        throw std::invalid_argument("Process ID must be non-negative (got " + std::to_string(pid) + ")");
    if (arrivalTime < 0)
        throw std::invalid_argument("Arrival time must be non-negative for P" + std::to_string(pid));
    if (burstTime <= 0)
        throw std::invalid_argument("Burst time must be positive for P" + std::to_string(pid));
}

void Process::resetSchedulingState() {
    completionTime = 0;
    turnaroundTime = 0;
    waitingTime    = 0;
    responseTime   = -1;
    remainingTime  = burstTime;
    executionIntervals.clear();
}

std::vector<Process> readProcesses(std::istream& in) {
    std::vector<Process> processes;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        int pid, arrival, burst, priority = 0;
        if (!(iss >> pid >> arrival >> burst)) continue;
        iss >> priority; // optional
        processes.emplace_back(pid, arrival, burst, priority);
    }
    return processes;
}

void printProcessTable(const std::vector<Process>& processes, std::ostream& out) {
    out << "\n";
    out << std::left
        << std::setw(6)  << "PID"
        << std::setw(10) << "Arrival"
        << std::setw(8)  << "Burst"
        << std::setw(10) << "Priority"
        << std::setw(12) << "Completion"
        << std::setw(12) << "Turnaround"
        << std::setw(10) << "Waiting"
        << std::setw(10) << "Response"
        << "\n";
    out << std::string(78, '-') << "\n";

    for (const auto& p : processes) {
        out << std::left
            << std::setw(6)  << p.pid
            << std::setw(10) << p.arrivalTime
            << std::setw(8)  << p.burstTime
            << std::setw(10) << p.priority
            << std::setw(12) << p.completionTime
            << std::setw(12) << p.turnaroundTime
            << std::setw(10) << p.waitingTime
            << std::setw(10) << p.responseTime
            << "\n";
    }
    out << "\n";
}

} // namespace SchedOS
