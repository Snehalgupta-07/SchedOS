#include "schedos/ComparisonEngine.h"
#include "schedos/FCFS.h"
#include "schedos/SJF.h"
#include "schedos/SRTF.h"
#include "schedos/RoundRobin.h"
#include "schedos/PriorityScheduling.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <climits>
#include <cfloat>

namespace SchedOS {

std::vector<ComparisonResult> ComparisonEngine::compareAll(const std::vector<Process>& processes,
                                                           int quantum,
                                                           int csCost) {
    std::vector<ComparisonResult> results;
    if (processes.empty()) return results;

    struct AlgDef {
        std::string key;
        std::unique_ptr<Scheduler> scheduler;
    };

    std::vector<AlgDef> algs;
    algs.push_back({"fcfs", std::make_unique<FCFS>()});
    algs.push_back({"sjf", std::make_unique<SJF>()});
    algs.push_back({"srtf", std::make_unique<SRTF>()});
    algs.push_back({"rr", std::make_unique<RoundRobin>(quantum)});
    algs.push_back({"priority", std::make_unique<PriorityScheduling>()});

    for (auto& item : algs) {
        auto procsCopy = processes;
        item.scheduler->setContextSwitchCost(csCost);
        item.scheduler->schedule(procsCopy);

        ComparisonResult res;
        res.algorithmKey       = item.key;
        res.algorithmName      = item.scheduler->name();
        res.contextSwitchCount = item.scheduler->contextSwitchCount();
        res.metrics            = SchedulingMetrics::compute(procsCopy,
                                                           item.scheduler->ganttChart(),
                                                           item.scheduler->contextSwitchCount(),
                                                           item.scheduler->totalContextSwitchOverhead());
        res.totalExecutionTime = res.metrics.totalSimulationDuration;
        results.push_back(res);
    }

    return results;
}

void ComparisonEngine::printComparisonTable(const std::vector<ComparisonResult>& results,
                                            std::ostream& out,
                                            bool useColors) {
    if (results.empty()) {
        out << "  [!] No comparison data available (empty process set).\n";
        return;
    }

    // Determine best values across algorithms
    double minAvgWT  = DBL_MAX;
    double minAvgTAT = DBL_MAX;
    double minAvgRT  = DBL_MAX;
    double maxCPU    = -1.0;
    double maxThru   = -1.0;
    int minCS        = INT_MAX;
    int minTotalTime = INT_MAX;

    for (const auto& r : results) {
        minAvgWT  = std::min(minAvgWT, r.metrics.avgWaitingTime);
        minAvgTAT = std::min(minAvgTAT, r.metrics.avgTurnaroundTime);
        minAvgRT  = std::min(minAvgRT, r.metrics.avgResponseTime);
        maxCPU    = std::max(maxCPU, r.metrics.productiveCpuUtilization);
        maxThru   = std::max(maxThru, r.metrics.throughput);
        minCS     = std::min(minCS, r.contextSwitchCount);
        minTotalTime = std::min(minTotalTime, r.totalExecutionTime);
    }

    const std::string green = useColors ? "\033[32m\033[1m" : "";
    const std::string cyan  = useColors ? "\033[36m\033[1m" : "";
    const std::string bold  = useColors ? "\033[1m" : "";
    const std::string reset = useColors ? "\033[0m" : "";

    out << "\n" << cyan << "====================================================================================================\n";
    out << "                             ALGORITHM PERFORMANCE COMPARISON BENCHMARK                             \n";
    out << "====================================================================================================\n" << reset;

    out << std::left
        << std::setw(38) << "Algorithm"
        << std::setw(10) << "Avg WT"
        << std::setw(10) << "Avg TAT"
        << std::setw(10) << "Avg RT"
        << std::setw(14) << "Prod. CPU(%)"
        << std::setw(14) << "Throughput"
        << std::setw(10) << "CS Count"
        << std::setw(10) << "Total Time"
        << "\n";
    out << std::string(106, '-') << "\n";

    for (const auto& r : results) {
        bool bestWT  = std::abs(r.metrics.avgWaitingTime - minAvgWT) < 0.001;
        bool bestTAT = std::abs(r.metrics.avgTurnaroundTime - minAvgTAT) < 0.001;
        bool bestRT  = std::abs(r.metrics.avgResponseTime - minAvgRT) < 0.001;
        bool bestCPU = std::abs(r.metrics.productiveCpuUtilization - maxCPU) < 0.001;
        bool bestThru = std::abs(r.metrics.throughput - maxThru) < 0.0001;
        bool bestCS  = (r.contextSwitchCount == minCS);
        bool bestTime = (r.totalExecutionTime == minTotalTime);

        out << std::left << std::setw(38) << r.algorithmName;

        // Avg WT
        if (bestWT) out << green << std::setw(10) << (std::to_string(r.metrics.avgWaitingTime).substr(0, 5) + "*") << reset;
        else out << std::setw(10) << std::fixed << std::setprecision(2) << r.metrics.avgWaitingTime;

        // Avg TAT
        if (bestTAT) out << green << std::setw(10) << (std::to_string(r.metrics.avgTurnaroundTime).substr(0, 5) + "*") << reset;
        else out << std::setw(10) << std::fixed << std::setprecision(2) << r.metrics.avgTurnaroundTime;

        // Avg RT
        if (bestRT) out << green << std::setw(10) << (std::to_string(r.metrics.avgResponseTime).substr(0, 5) + "*") << reset;
        else out << std::setw(10) << std::fixed << std::setprecision(2) << r.metrics.avgResponseTime;

        // CPU Util
        if (bestCPU) out << green << std::setw(14) << (std::to_string(r.metrics.productiveCpuUtilization).substr(0, 5) + "%*") << reset;
        else out << std::setw(14) << (std::to_string(r.metrics.productiveCpuUtilization).substr(0, 5) + "%");

        // Throughput
        if (bestThru) out << green << std::setw(14) << (std::to_string(r.metrics.throughput).substr(0, 6) + "*") << reset;
        else out << std::setw(14) << std::fixed << std::setprecision(4) << r.metrics.throughput;

        // CS Count
        if (bestCS) out << green << std::setw(10) << (std::to_string(r.contextSwitchCount) + "*") << reset;
        else out << std::setw(10) << r.contextSwitchCount;

        // Total Time
        if (bestTime) out << green << std::setw(10) << (std::to_string(r.totalExecutionTime) + "*") << reset;
        else out << std::setw(10) << r.totalExecutionTime;

        out << "\n";
    }

    out << std::string(100, '-') << "\n";
    out << "  " << green << "* Indicates best performance for that metric." << reset << "\n\n";

    printTradeoffAnalysis(out, useColors);
}

void ComparisonEngine::printTradeoffAnalysis(std::ostream& out, bool useColors) {
    const std::string yellow = useColors ? "\033[33m\033[1m" : "";
    const std::string bold   = useColors ? "\033[1m" : "";
    const std::string reset  = useColors ? "\033[0m" : "";

    out << yellow << "=== Architectural Trade-off Analysis (Operating System Principles) ===\n" << reset;
    out << bold << "1. First-Come, First-Served (FCFS):\n" << reset
        << "   - Pros: Minimal scheduling overhead (0 preemption); starvation-free.\n"
        << "   - Cons: Susceptible to Convoy Effect; higher average turnaround time when long bursts arrive early.\n\n"
        << bold << "2. Shortest Job First (SJF - Non-Preemptive):\n" << reset
        << "   - Pros: Mathematically provable minimum average waiting time for static batch arrivals.\n"
        << "   - Cons: Potential starvation for long-running jobs; requires burst-time knowledge.\n\n"
        << bold << "3. Shortest Remaining Time First (SRTF - Preemptive):\n" << reset
        << "   - Pros: Highly responsive to newly arrived short bursts; reduces average turnaround time dynamically.\n"
        << "   - Cons: High context-switching overhead under frequent arrivals; risk of starvation.\n\n"
        << bold << "4. Round Robin (RR - Preemptive):\n" << reset
        << "   - Pros: Optimal for interactive time-sharing systems; guaranteed bounded response time.\n"
        << "   - Cons: Overhead highly sensitive to quantum size 'q' (too small = excessive CS, too large = degrades to FCFS).\n\n"
        << bold << "5. Priority Scheduling (Non-Preemptive):\n" << reset
        << "   - Pros: Directly aligns CPU allocation with real-world task criticalities.\n"
        << "   - Cons: Lower-priority jobs face indefinite postponement (requires aging techniques in production).\n\n"
        << "Summary: No single scheduling algorithm is universally superior. Production OS kernels\n"
        << "(e.g., Linux CFS or Siemens industrial RTOS) combine these strategies into multi-level priority queues.\n\n";
}

} // namespace SchedOS
