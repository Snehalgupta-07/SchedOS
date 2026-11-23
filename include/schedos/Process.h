#pragma once

#include <string>
#include <vector>
#include <utility>
#include <iosfwd>
#include <stdexcept>

namespace SchedOS {

/// Represents a single process in the scheduling simulation.
struct Process {
    int pid;            // Process ID
    int arrivalTime;    // Time at which the process arrives
    int burstTime;      // Total CPU burst time required
    int priority;       // Priority (lower value = higher priority)

    // --- Computed at scheduling time ---
    int completionTime = 0;
    int turnaroundTime = 0;
    int waitingTime    = 0;
    int responseTime   = -1;   // -1 indicates not yet scheduled

    int remainingTime  = 0;    // Used for preemptive algorithms

    // Gantt chart intervals: (start_time, end_time)
    std::vector<std::pair<int, int>> executionIntervals;

    Process(int id, int arrival, int burst, int prio = 0);

    /// Validates process fields; throws std::invalid_argument on bad data.
    void validate() const;

    /// Resets scheduling results so the process can be re-scheduled.
    void resetSchedulingState();
};

/// Reads a list of processes from an input stream (one per line: pid arrival burst priority).
std::vector<Process> readProcesses(std::istream& in);

/// Prints a formatted table of process results.
void printProcessTable(const std::vector<Process>& processes, std::ostream& out);

} // namespace SchedOS
