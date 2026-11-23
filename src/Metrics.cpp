#include "schedos/Metrics.h"
#include <iostream>
#include <iomanip>
#include <numeric>

namespace SchedOS {

SchedulingMetrics SchedulingMetrics::compute(const std::vector<Process>& processes,
                                              const std::vector<GanttEntry>& gantt) {
    SchedulingMetrics m;
    if (processes.empty()) return m;

    int n = static_cast<int>(processes.size());

    double totalWaiting    = 0.0;
    double totalTurnaround = 0.0;
    double totalResponse   = 0.0;

    for (const auto& p : processes) {
        totalWaiting    += p.waitingTime;
        totalTurnaround += p.turnaroundTime;
        totalResponse   += p.responseTime;
    }

    m.avgWaitingTime    = totalWaiting / n;
    m.avgTurnaroundTime = totalTurnaround / n;
    m.avgResponseTime   = totalResponse / n;

    if (!gantt.empty()) {
        int totalTime = 0;
        int busyTime  = 0;
        int minStart  = std::get<1>(gantt.front());
        int maxEnd    = std::get<2>(gantt.back());

        for (const auto& [pid, start, end] : gantt) {
            if (pid != -1) {
                busyTime += (end - start);
            }
        }

        totalTime = maxEnd - minStart;
        if (totalTime > 0) {
            m.cpuUtilization = (static_cast<double>(busyTime) / totalTime) * 100.0;
            m.throughput     = static_cast<double>(n) / totalTime;
        }
    }

    return m;
}

void SchedulingMetrics::print(std::ostream& out) const {
    out << std::fixed << std::setprecision(2);
    out << "\n=== Scheduling Metrics ===\n";
    out << "Average Waiting Time    : " << avgWaitingTime    << "\n";
    out << "Average Turnaround Time : " << avgTurnaroundTime << "\n";
    out << "Average Response Time   : " << avgResponseTime   << "\n";
    out << "CPU Utilization         : " << cpuUtilization    << "%\n";
    out << "Throughput              : " << throughput         << " processes/unit time\n";
    out << "\n";
}

} // namespace SchedOS
