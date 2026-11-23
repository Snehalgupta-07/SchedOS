#pragma once

#include "schedos/Process.h"
#include "schedos/Scheduler.h"
#include <vector>
#include <iosfwd>

namespace SchedOS {

/// Computes and stores aggregate scheduling metrics.
struct SchedulingMetrics {
    double avgWaitingTime    = 0.0;
    double avgTurnaroundTime = 0.0;
    double avgResponseTime   = 0.0;
    double cpuUtilization    = 0.0;  // Percentage
    double throughput        = 0.0;  // Processes per unit time

    /// Computes metrics from a list of already-scheduled processes and a Gantt chart.
    static SchedulingMetrics compute(const std::vector<Process>& processes,
                                     const std::vector<GanttEntry>& gantt);

    /// Pretty-prints the metrics.
    void print(std::ostream& out) const;
};

} // namespace SchedOS
