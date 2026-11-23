#include "schedos/Metrics.h"
#include <iostream>
#include <iomanip>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <climits>

namespace SchedOS {

SchedulingMetrics SchedulingMetrics::compute(const std::vector<Process>& processes,
                                             const std::vector<GanttEntry>& gantt,
                                             int csCount,
                                             int csOverhead) {
    SchedulingMetrics m;
    if (processes.empty()) return m;

    int n = static_cast<int>(processes.size());

    double totalWaiting    = 0.0;
    double totalTurnaround = 0.0;
    double totalResponse   = 0.0;

    int minWT = INT_MAX;
    int maxWT = INT_MIN;

    for (const auto& p : processes) {
        totalWaiting    += p.waitingTime;
        totalTurnaround += p.turnaroundTime;
        totalResponse   += p.responseTime;

        minWT = std::min(minWT, p.waitingTime);
        maxWT = std::max(maxWT, p.waitingTime);
    }

    m.avgWaitingTime    = totalWaiting / n;
    m.avgTurnaroundTime = totalTurnaround / n;
    m.avgResponseTime   = totalResponse / n;

    m.minWaitingTime = (minWT == INT_MAX) ? 0 : minWT;
    m.maxWaitingTime = (maxWT == INT_MIN) ? 0 : maxWT;

    // Calculate Waiting-Time Variance and Standard Deviation
    double varSum = 0.0;
    for (const auto& p : processes) {
        double diff = static_cast<double>(p.waitingTime) - m.avgWaitingTime;
        varSum += diff * diff;
    }
    m.waitingTimeVariance = varSum / n;
    m.waitingTimeStdDev   = std::sqrt(m.waitingTimeVariance);

    if (!gantt.empty()) {
        int minStart  = std::get<1>(gantt.front());
        int maxEnd    = std::get<2>(gantt.back());

        int busyTime = 0;
        int idleTime = 0;
        int derivedCsOverhead = 0;
        int derivedCsCount = 0;

        for (const auto& [pid, start, end] : gantt) {
            int duration = end - start;
            if (pid >= 0) {
                busyTime += duration;
            } else if (pid == -1) {
                idleTime += duration;
            } else if (pid == -2) {
                derivedCsOverhead += duration;
                derivedCsCount++;
            }
        }

        m.totalBusyTime              = busyTime;
        m.totalIdleTime              = idleTime;
        m.totalContextSwitchOverhead = (csOverhead >= 0) ? csOverhead : derivedCsOverhead;
        m.contextSwitchCount         = (csCount >= 0) ? csCount : derivedCsCount;
        m.totalSimulationDuration    = maxEnd - minStart;

        if (m.totalSimulationDuration > 0) {
            m.productiveCpuUtilization = (static_cast<double>(m.totalBusyTime) / m.totalSimulationDuration) * 100.0;
            m.cpuUtilization           = m.productiveCpuUtilization;
            m.totalCpuOccupancy        = (static_cast<double>(m.totalBusyTime + m.totalContextSwitchOverhead) / m.totalSimulationDuration) * 100.0;
            m.throughput               = static_cast<double>(n) / m.totalSimulationDuration;
        }
    }

    return m;
}

void SchedulingMetrics::print(std::ostream& out) const {
    out << std::fixed << std::setprecision(2);
    out << "\n=== Comprehensive Scheduling & CPU Metrics ===\n";
    out << "  Total Simulation Time    : " << totalSimulationDuration << " time units\n";
    out << "  Total CPU Busy Time      : " << totalBusyTime << " time units\n";
    out << "  Total CPU Idle Time      : " << totalIdleTime << " time units\n";
    out << "  Context-Switch Overhead  : " << totalContextSwitchOverhead << " time units (" << contextSwitchCount << " switches)\n";
    out << "  --------------------------------------------------\n";
    out << "  Average Waiting Time     : " << avgWaitingTime    << "\n";
    out << "  Average Turnaround Time  : " << avgTurnaroundTime << "\n";
    out << "  Average Response Time    : " << avgResponseTime   << "\n";
    out << "  Productive CPU Util.     : " << productiveCpuUtilization << "%\n";
    out << "  Total CPU Occupancy      : " << totalCpuOccupancy << "%\n";
    out << "  Throughput               : " << throughput         << " processes/unit time\n";
    out << "  --------------------------------------------------\n";
    out << "  Fairness & Dispersion Analysis:\n";
    out << "    Minimum Waiting Time   : " << minWaitingTime << " time units\n";
    out << "    Maximum Waiting Time   : " << maxWaitingTime << " time units\n";
    out << "    Waiting-Time Variance  : " << waitingTimeVariance << "\n";
    out << "    Waiting-Time Std. Dev. : " << waitingTimeStdDev << "\n";
    out << "    (Note: Lower variance/std. dev. indicates more uniform waiting time distribution)\n";
    out << "\n";
}

} // namespace SchedOS
