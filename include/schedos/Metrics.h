#pragma once

#include "schedos/Process.h"
#include "schedos/Scheduler.h"
#include <vector>
#include <iosfwd>

namespace SchedOS {

/// Computes and stores comprehensive scheduling, CPU performance, and fairness metrics.
struct SchedulingMetrics {
    // Standard OS process metrics
    double avgWaitingTime    = 0.0;
    double avgTurnaroundTime = 0.0;
    double avgResponseTime   = 0.0;

    // CPU utilization & throughput
    double cpuUtilization            = 0.0;  // Productive CPU utilization (%)
    double productiveCpuUtilization  = 0.0;  // (Busy Time / Total Time) * 100
    double totalCpuOccupancy         = 0.0;  // ((Busy Time + CS Overhead) / Total Time) * 100
    double throughput                = 0.0;  // Processes per unit time

    // Fairness & Waiting-Time Dispersion
    int maxWaitingTime               = 0;
    int minWaitingTime               = 0;
    double waitingTimeVariance       = 0.0;
    double waitingTimeStdDev         = 0.0;

    // Simulation timeline totals
    int totalSimulationDuration      = 0;
    int totalBusyTime                = 0;
    int totalIdleTime                = 0;
    int totalContextSwitchOverhead   = 0;
    int contextSwitchCount           = 0;

    /// Computes metrics from scheduled processes and Gantt chart entries.
    static SchedulingMetrics compute(const std::vector<Process>& processes,
                                     const std::vector<GanttEntry>& gantt,
                                     int csCount = -1,
                                     int csOverhead = -1);

    /// Formatted output of all metrics.
    void print(std::ostream& out) const;
};

} // namespace SchedOS
