// SchedOS — Lightweight test harness (no external dependencies)
// Tests all five scheduling algorithms with known expected results.

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <cmath>
#include <string>
#include <sstream>
#include <functional>

#include "schedos/Process.h"
#include "schedos/ProcessManager.h"
#include "schedos/GanttChart.h"
#include "schedos/ComparisonEngine.h"
#include "schedos/CSVManager.h"
#include "schedos/TaskQueue.h"
#include "schedos/ThreadPool.h"
#include "schedos/BenchmarkEngine.h"
#include "schedos/FCFS.h"
#include "schedos/SJF.h"
#include "schedos/SRTF.h"
#include "schedos/RoundRobin.h"
#include "schedos/PriorityScheduling.h"
#include "schedos/Metrics.h"

using namespace SchedOS;

static int totalTests  = 0;
static int passedTests = 0;
static int failedTests = 0;

#define ASSERT_EQ(actual, expected, msg)                                     \
    do {                                                                      \
        ++totalTests;                                                         \
        if ((actual) == (expected)) {                                          \
            ++passedTests;                                                    \
        } else {                                                              \
            ++failedTests;                                                    \
            std::cerr << "FAIL: " << (msg)                                    \
                      << " | expected=" << (expected)                         \
                      << " actual=" << (actual)                               \
                      << " (" << __FILE__ << ":" << __LINE__ << ")\n";       \
        }                                                                     \
    } while (0)

#define ASSERT_NEAR(actual, expected, eps, msg)                              \
    do {                                                                      \
        ++totalTests;                                                         \
        if (std::fabs((actual) - (expected)) <= (eps)) {                      \
            ++passedTests;                                                    \
        } else {                                                              \
            ++failedTests;                                                    \
            std::cerr << "FAIL: " << (msg)                                    \
                      << " | expected≈" << (expected)                        \
                      << " actual=" << (actual)                               \
                      << " (" << __FILE__ << ":" << __LINE__ << ")\n";       \
        }                                                                     \
    } while (0)

#define ASSERT_THROW(expr, exType, msg)                                      \
    do {                                                                      \
        ++totalTests;                                                         \
        bool caught = false;                                                  \
        try { expr; } catch (const exType&) { caught = true; }               \
        if (caught) { ++passedTests; }                                       \
        else {                                                                \
            ++failedTests;                                                    \
            std::cerr << "FAIL: " << (msg)                                    \
                      << " | expected exception not thrown"                    \
                      << " (" << __FILE__ << ":" << __LINE__ << ")\n";       \
        }                                                                     \
    } while (0)

void runSection(const std::string& name, std::function<void()> fn) {
    std::cout << "--- " << name << " ---\n";
    fn();
}

std::vector<Process> makeStandard3() {
    return { Process(1, 0, 5, 2), Process(2, 1, 3, 1), Process(3, 2, 8, 3) };
}

void testFCFS_basic() {
    auto procs = makeStandard3();
    FCFS fcfs;
    fcfs.schedule(procs);

    ASSERT_EQ(procs[0].completionTime, 5,  "FCFS P1 CT");
    ASSERT_EQ(procs[1].completionTime, 8,  "FCFS P2 CT");
    ASSERT_EQ(procs[2].completionTime, 16, "FCFS P3 CT");
    ASSERT_EQ(procs[0].waitingTime, 0, "FCFS P1 WT");
    ASSERT_EQ(procs[1].waitingTime, 4, "FCFS P2 WT");
    ASSERT_EQ(procs[2].waitingTime, 6, "FCFS P3 WT");
    ASSERT_EQ(procs[0].responseTime, 0, "FCFS P1 RT");
    ASSERT_EQ(procs[1].responseTime, 4, "FCFS P2 RT");
    ASSERT_EQ(procs[2].responseTime, 6, "FCFS P3 RT");
}

void testFCFS_simultaneousArrival() {
    std::vector<Process> procs = { Process(1, 0, 4), Process(2, 0, 2), Process(3, 0, 6) };
    FCFS fcfs;
    fcfs.schedule(procs);

    ASSERT_EQ(procs[0].completionTime, 4,  "FCFS simult P1 CT");
    ASSERT_EQ(procs[1].completionTime, 6,  "FCFS simult P2 CT");
    ASSERT_EQ(procs[2].completionTime, 12, "FCFS simult P3 CT");
}

void testFCFS_idlePeriod() {
    std::vector<Process> procs = { Process(1, 0, 2), Process(2, 5, 3) };
    FCFS fcfs;
    fcfs.schedule(procs);

    ASSERT_EQ(procs[0].completionTime, 2, "FCFS idle P1 CT");
    ASSERT_EQ(procs[1].completionTime, 8, "FCFS idle P2 CT");
    ASSERT_EQ(procs[1].waitingTime, 0,    "FCFS idle P2 WT");

    bool hasIdle = false;
    for (const auto& [pid, s, e] : fcfs.ganttChart()) {
        if (pid == -1) { hasIdle = true; break; }
    }
    ASSERT_EQ(hasIdle, true, "FCFS Gantt has idle period");
}

void testFCFS_singleProcess() {
    std::vector<Process> procs = { Process(1, 0, 7) };
    FCFS fcfs;
    fcfs.schedule(procs);

    ASSERT_EQ(procs[0].completionTime, 7, "FCFS single CT");
    ASSERT_EQ(procs[0].waitingTime, 0,    "FCFS single WT");
    ASSERT_EQ(procs[0].responseTime, 0,   "FCFS single RT");
}

void testSJF_basic() {
    auto procs = makeStandard3();
    SJF sjf;
    sjf.schedule(procs);

    ASSERT_EQ(procs[0].completionTime, 5,  "SJF P1 CT");
    ASSERT_EQ(procs[1].completionTime, 8,  "SJF P2 CT");
    ASSERT_EQ(procs[2].completionTime, 16, "SJF P3 CT");
    ASSERT_EQ(procs[1].waitingTime, 4,     "SJF P2 WT");
}

void testSJF_tieBreaking() {
    std::vector<Process> procs = { Process(1, 0, 3), Process(2, 0, 3) };
    SJF sjf;
    sjf.schedule(procs);

    ASSERT_EQ(procs[0].completionTime, 3, "SJF tie P1 CT");
    ASSERT_EQ(procs[1].completionTime, 6, "SJF tie P2 CT");
}

void testSRTF_basic() {
    auto procs = makeStandard3();
    SRTF srtf;
    srtf.schedule(procs);

    ASSERT_EQ(procs[0].completionTime, 8,  "SRTF P1 CT");
    ASSERT_EQ(procs[1].completionTime, 4,  "SRTF P2 CT");
    ASSERT_EQ(procs[2].completionTime, 16, "SRTF P3 CT");

    ASSERT_EQ(procs[0].waitingTime, 3,  "SRTF P1 WT");
    ASSERT_EQ(procs[1].waitingTime, 0,  "SRTF P2 WT");
    ASSERT_EQ(procs[2].waitingTime, 6,  "SRTF P3 WT");

    ASSERT_EQ(procs[0].responseTime, 0, "SRTF P1 RT");
    ASSERT_EQ(procs[1].responseTime, 0, "SRTF P2 RT");
    ASSERT_EQ(procs[2].responseTime, 6, "SRTF P3 RT");
}

void testSRTF_noPreemption() {
    std::vector<Process> procs = { Process(1, 0, 5), Process(2, 0, 2), Process(3, 0, 8) };
    SRTF srtf;
    srtf.schedule(procs);

    ASSERT_EQ(procs[1].completionTime, 2,  "SRTF no-preempt P2 CT");
    ASSERT_EQ(procs[0].completionTime, 7,  "SRTF no-preempt P1 CT");
    ASSERT_EQ(procs[2].completionTime, 15, "SRTF no-preempt P3 CT");
}

void testRR_basic() {
    auto procs = makeStandard3();
    RoundRobin rr(2);
    rr.schedule(procs);

    ASSERT_EQ(procs[0].completionTime, 12, "RR P1 CT");
    ASSERT_EQ(procs[1].completionTime, 9,  "RR P2 CT");
    ASSERT_EQ(procs[2].completionTime, 16, "RR P3 CT");

    ASSERT_EQ(procs[0].responseTime, 0, "RR P1 RT");
    ASSERT_EQ(procs[1].responseTime, 1, "RR P2 RT");
    ASSERT_EQ(procs[2].responseTime, 2, "RR P3 RT");
}

void testRR_quantum1() {
    std::vector<Process> procs = { Process(1, 0, 3), Process(2, 0, 2) };
    RoundRobin rr(1);
    rr.schedule(procs);

    ASSERT_EQ(procs[0].completionTime, 5, "RR q1 P1 CT");
    ASSERT_EQ(procs[1].completionTime, 4, "RR q1 P2 CT");
}

void testRR_quantumLargerThanBurst() {
    std::vector<Process> procs = { Process(1, 0, 3), Process(2, 1, 2) };
    RoundRobin rr(10);
    rr.schedule(procs);

    ASSERT_EQ(procs[0].completionTime, 3, "RR big-q P1 CT");
    ASSERT_EQ(procs[1].completionTime, 5, "RR big-q P2 CT");
}

void testRR_invalidQuantum() {
    ASSERT_THROW(RoundRobin rr(0), std::invalid_argument, "RR quantum=0 throws");
    ASSERT_THROW(RoundRobin rr(-1), std::invalid_argument, "RR quantum=-1 throws");
}

void testPriority_basic() {
    auto procs = makeStandard3();
    PriorityScheduling ps;
    ps.schedule(procs);

    ASSERT_EQ(procs[0].completionTime, 5,  "Priority P1 CT");
    ASSERT_EQ(procs[1].completionTime, 8,  "Priority P2 CT");
    ASSERT_EQ(procs[2].completionTime, 16, "Priority P3 CT");
}

void testPriority_samePriority() {
    std::vector<Process> procs = { Process(1, 0, 4, 1), Process(2, 1, 3, 1), Process(3, 2, 2, 1) };
    PriorityScheduling ps;
    ps.schedule(procs);

    ASSERT_EQ(procs[0].completionTime, 4, "Priority same P1 CT");
    ASSERT_EQ(procs[1].completionTime, 7, "Priority same P2 CT");
    ASSERT_EQ(procs[2].completionTime, 9, "Priority same P3 CT");
}

void testMetrics_basic() {
    auto procs = makeStandard3();
    FCFS fcfs;
    fcfs.schedule(procs);

    auto m = SchedulingMetrics::compute(procs, fcfs.ganttChart());
    ASSERT_NEAR(m.avgWaitingTime, 10.0 / 3.0, 0.01, "Metrics avg WT");
    ASSERT_NEAR(m.avgTurnaroundTime, 26.0 / 3.0, 0.01, "Metrics avg TAT");
    ASSERT_NEAR(m.cpuUtilization, 100.0, 0.01, "Metrics CPU util");
    ASSERT_NEAR(m.throughput, 3.0 / 16.0, 0.01, "Metrics throughput");
}

void testMetrics_withIdle() {
    std::vector<Process> procs = { Process(1, 0, 2), Process(2, 5, 3) };
    FCFS fcfs;
    fcfs.schedule(procs);

    auto m = SchedulingMetrics::compute(procs, fcfs.ganttChart());
    ASSERT_NEAR(m.cpuUtilization, 62.5, 0.01, "Metrics idle CPU util");
}

void testInvalidProcess() {
    ASSERT_THROW(Process(-1, 0, 5), std::invalid_argument, "Negative PID");
    ASSERT_THROW(Process(1, -1, 5), std::invalid_argument, "Negative arrival");
    ASSERT_THROW(Process(1, 0, 0),  std::invalid_argument, "Zero burst");
    ASSERT_THROW(Process(1, 0, -3), std::invalid_argument, "Negative burst");
}

void testEmptySchedule() {
    std::vector<Process> procs;
    FCFS fcfs;
    fcfs.schedule(procs);
    ASSERT_EQ(fcfs.ganttChart().empty(), true, "Empty schedule Gantt empty");
}

void testReadProcesses() {
    std::istringstream input("1 0 5 2\n# comment\n2 1 3 1\n\n3 2 8 3\n");
    auto procs = readProcesses(input);
    ASSERT_EQ(static_cast<int>(procs.size()), 3, "readProcesses count");
    ASSERT_EQ(procs[0].pid, 1, "readProcesses P1 pid");
    ASSERT_EQ(procs[1].burstTime, 3, "readProcesses P2 burst");
    ASSERT_EQ(procs[2].priority, 3, "readProcesses P3 priority");
}

void testProcessManager() {
    ProcessManager pm;
    ASSERT_EQ(pm.empty(), true, "PM initially empty");
    ASSERT_EQ(static_cast<int>(pm.size()), 0, "PM size 0");

    pm.addProcess(Process(1, 0, 5, 2));
    pm.addProcess(Process(2, 2, 4, 1));
    ASSERT_EQ(static_cast<int>(pm.size()), 2, "PM size 2 after adds");
    ASSERT_EQ(pm.hasPid(1), true, "PM has P1");
    ASSERT_EQ(pm.hasPid(99), false, "PM does not have P99");

    // Duplicate PID rejection
    ASSERT_THROW(pm.addProcess(Process(1, 1, 3, 0)), std::invalid_argument, "PM reject duplicate PID");

    // Edit process
    bool edited = pm.editProcess(1, 0, 8, 3);
    ASSERT_EQ(edited, true, "PM edit existing P1");
    auto p1 = pm.findProcess(1);
    ASSERT_EQ(p1.has_value(), true, "PM found P1");
    ASSERT_EQ(p1->burstTime, 8, "PM P1 burst updated");

    bool editNonexistent = pm.editProcess(99, 0, 5, 0);
    ASSERT_EQ(editNonexistent, false, "PM edit nonexistent");

    // Delete process
    bool deleted = pm.deleteProcess(2);
    ASSERT_EQ(deleted, true, "PM delete P2");
    ASSERT_EQ(static_cast<int>(pm.size()), 1, "PM size 1 after delete");
    ASSERT_EQ(pm.hasPid(2), false, "PM P2 deleted");

    bool deleteNonexistent = pm.deleteProcess(99);
    ASSERT_EQ(deleteNonexistent, false, "PM delete nonexistent");

    // Clear
    pm.clear();
    ASSERT_EQ(pm.empty(), true, "PM empty after clear");
}

void testGanttChart() {
    // 1. mergeAdjacent
    std::vector<GanttEntry> raw = {
        {1, 0, 2},
        {1, 2, 5},
        {2, 5, 8},
        {1, 8, 10},
        {1, 10, 12},
        {-1, 12, 15},
        {-1, 15, 18}
    };
    auto merged = GanttChart::mergeAdjacent(raw);
    ASSERT_EQ(static_cast<int>(merged.size()), 4, "Gantt mergeAdjacent count");
    ASSERT_EQ(std::get<0>(merged[0]), 1, "Gantt merged[0] pid");
    ASSERT_EQ(std::get<1>(merged[0]), 0, "Gantt merged[0] start");
    ASSERT_EQ(std::get<2>(merged[0]), 5, "Gantt merged[0] end");

    ASSERT_EQ(std::get<0>(merged[1]), 2, "Gantt merged[1] pid");
    ASSERT_EQ(std::get<2>(merged[1]), 8, "Gantt merged[1] end");

    ASSERT_EQ(std::get<0>(merged[2]), 1, "Gantt merged[2] pid");
    ASSERT_EQ(std::get<2>(merged[2]), 12, "Gantt merged[2] end");

    ASSERT_EQ(std::get<0>(merged[3]), -1, "Gantt merged[3] pid (idle)");
    ASSERT_EQ(std::get<2>(merged[3]), 18, "Gantt merged[3] end");

    // 2. renderTimeline
    std::ostringstream oss;
    GanttChart::renderTimeline(merged, oss, false);
    std::string outStr = oss.str();
    bool hasP1 = outStr.find("P1 [0-5]") != std::string::npos;
    bool hasP2 = outStr.find("P2 [5-8]") != std::string::npos;
    bool hasIdle = outStr.find("IDLE [12-18]") != std::string::npos;
    ASSERT_EQ(hasP1, true, "Gantt timeline has P1 [0-5]");
    ASSERT_EQ(hasP2, true, "Gantt timeline has P2 [5-8]");
    ASSERT_EQ(hasIdle, true, "Gantt timeline has IDLE [12-18]");

    // 3. renderBarChart does not crash and outputs content
    std::ostringstream barOss;
    GanttChart::renderBarChart(merged, barOss, 80, false);
    ASSERT_EQ(!barOss.str().empty(), true, "Gantt bar chart non-empty");
}

void testContextSwitching() {
    // 1. FCFS with context switch cost = 2
    // P1(0, 5), P2(0, 5)
    // P1: runs 0 to 5.
    // CS: 5 to 7 (overhead = 2).
    // P2: runs 7 to 12.
    std::vector<Process> procs = { Process(1, 0, 5), Process(2, 0, 5) };
    FCFS fcfs;
    fcfs.setContextSwitchCost(2);
    fcfs.schedule(procs);

    ASSERT_EQ(fcfs.contextSwitchCount(), 1, "FCFS CS count");
    ASSERT_EQ(fcfs.totalContextSwitchOverhead(), 2, "FCFS CS overhead");
    ASSERT_EQ(procs[0].completionTime, 5, "FCFS CS P1 CT");
    ASSERT_EQ(procs[1].completionTime, 12, "FCFS CS P2 CT");
    ASSERT_EQ(procs[1].waitingTime, 7, "FCFS CS P2 WT");

    auto m = SchedulingMetrics::compute(procs, fcfs.ganttChart(), fcfs.contextSwitchCount(), fcfs.totalContextSwitchOverhead());
    ASSERT_EQ(m.totalBusyTime, 10, "FCFS CS busy time");
    ASSERT_EQ(m.totalContextSwitchOverhead, 2, "FCFS CS total overhead in metrics");
    ASSERT_EQ(m.totalSimulationDuration, 12, "FCFS CS simulation duration");
    // Productive utilization: 10/12 ≈ 83.33%
    ASSERT_NEAR(m.productiveCpuUtilization, (10.0 / 12.0) * 100.0, 0.01, "FCFS productive CPU util");
    // Total occupancy: (10+2)/12 = 100%
    ASSERT_NEAR(m.totalCpuOccupancy, 100.0, 0.01, "FCFS total CPU occupancy");

    // 2. Round Robin with context switch cost = 1
    // P1(0, 4), P2(0, 4), quantum = 2, csCost = 1
    // t=0-2: P1
    // t=2-3: CS (overhead 1)
    // t=3-5: P2
    // t=5-6: CS (overhead 1)
    // t=6-8: P1 (finishes at 8)
    // t=8-9: CS (overhead 1)
    // t=9-11: P2 (finishes at 11)
    std::vector<Process> rrProcs = { Process(1, 0, 4), Process(2, 0, 4) };
    RoundRobin rr(2);
    rr.setContextSwitchCost(1);
    rr.schedule(rrProcs);

    ASSERT_EQ(rr.contextSwitchCount(), 3, "RR CS count");
    ASSERT_EQ(rr.totalContextSwitchOverhead(), 3, "RR CS overhead");
    ASSERT_EQ(rrProcs[0].completionTime, 8, "RR CS P1 CT");
    ASSERT_EQ(rrProcs[1].completionTime, 11, "RR CS P2 CT");

    // 3. Arrival during context switch:
    // P1 arrives at 0 (burst 3), P2 arrives at 4 (burst 2)
    // CS cost = 2.
    // P1 finishes at 3. CPU idle until P2 arrives at 4.
    // Since CPU was idle from 3 to 4, waking from idle does not charge an inter-process context switch.
    std::vector<Process> idleProcs = { Process(1, 0, 3), Process(2, 4, 2) };
    FCFS fcfsIdle;
    fcfsIdle.setContextSwitchCost(2);
    fcfsIdle.schedule(idleProcs);
    ASSERT_EQ(fcfsIdle.contextSwitchCount(), 0, "Idle to run no inter-process CS");
    ASSERT_EQ(idleProcs[1].completionTime, 6, "Idle to run P2 CT");
}

void testComparisonEngine() {
    auto procs = makeStandard3();
    auto results = ComparisonEngine::compareAll(procs, 2, 0);

    ASSERT_EQ(static_cast<int>(results.size()), 5, "CompareAll returns 5 algorithms");
    ASSERT_EQ(results[0].algorithmKey, "fcfs", "CompareAll[0] is fcfs");
    ASSERT_EQ(results[1].algorithmKey, "sjf", "CompareAll[1] is sjf");
    ASSERT_EQ(results[2].algorithmKey, "srtf", "CompareAll[2] is srtf");
    ASSERT_EQ(results[3].algorithmKey, "rr", "CompareAll[3] is rr");
    ASSERT_EQ(results[4].algorithmKey, "priority", "CompareAll[4] is priority");

    // Check that SRTF has lower or equal avg waiting time than FCFS on standard3
    ASSERT_EQ(results[2].metrics.avgWaitingTime <= results[0].metrics.avgWaitingTime, true, "SRTF avg WT <= FCFS avg WT");

    // Test printComparisonTable output
    std::ostringstream oss;
    ComparisonEngine::printComparisonTable(results, oss, false);
    ASSERT_EQ(!oss.str().empty(), true, "Comparison table non-empty");
    ASSERT_EQ(oss.str().find("ALGORITHM PERFORMANCE COMPARISON BENCHMARK") != std::string::npos, true, "Comparison table header present");
}

void testCSVManager() {
    // 1. Valid import (supports both root cwd and build/ cwd)
    std::string samplePath = "samples/processes.csv";
    if (!std::ifstream(samplePath).good()) {
        samplePath = "../samples/processes.csv";
    }
    auto procs = CSVManager::importProcesses(samplePath);
    ASSERT_EQ(static_cast<int>(procs.size()), 5, "CSV import 5 procs");
    ASSERT_EQ(procs[0].pid, 1, "CSV P1 pid");
    ASSERT_EQ(procs[0].burstTime, 8, "CSV P1 burst");
    ASSERT_EQ(procs[4].pid, 5, "CSV P5 pid");
    ASSERT_EQ(procs[4].priority, 0, "CSV P5 priority");

    // 2. Non-existent file
    ASSERT_THROW(CSVManager::importProcesses("samples/nonexistent_file.csv"), std::runtime_error, "CSV nonexistent file");

    // 3. Validation on temp CSV files
    {
        // Duplicate PID
        std::ofstream out("test_dup.csv");
        out << "PID,Arrival,Burst,Priority\n1,0,5,1\n1,2,3,2\n";
        out.close();
        ASSERT_THROW(CSVManager::importProcesses("test_dup.csv"), std::invalid_argument, "CSV duplicate PID");
    }
    {
        // Negative arrival
        std::ofstream out("test_neg_arr.csv");
        out << "PID,Arrival,Burst,Priority\n1,-5,5,1\n";
        out.close();
        ASSERT_THROW(CSVManager::importProcesses("test_neg_arr.csv"), std::invalid_argument, "CSV negative arrival");
    }
    {
        // Zero burst
        std::ofstream out("test_zero_burst.csv");
        out << "PID,Arrival,Burst,Priority\n1,0,0,1\n";
        out.close();
        ASSERT_THROW(CSVManager::importProcesses("test_zero_burst.csv"), std::invalid_argument, "CSV zero burst");
    }
    {
        // Insufficient columns
        std::ofstream out("test_bad_cols.csv");
        out << "PID,Arrival\n1,0\n";
        out.close();
        ASSERT_THROW(CSVManager::importProcesses("test_bad_cols.csv"), std::invalid_argument, "CSV bad columns");
    }

    // 4. Export results and verify file
    {
        FCFS fcfs;
        fcfs.schedule(procs);
        auto m = SchedulingMetrics::compute(procs, fcfs.ganttChart());
        CSVManager::exportScheduleResults("test_export.csv", fcfs.name(), procs, m);

        std::ifstream in("test_export.csv");
        ASSERT_EQ(in.is_open(), true, "CSV export file created");
        std::string firstLine;
        std::getline(in, firstLine);
        ASSERT_EQ(firstLine.find("# SchedOS Simulation Results") != std::string::npos, true, "CSV export header correct");
    }

    // 5. Export comparison results
    {
        auto compResults = ComparisonEngine::compareAll(procs, 2, 0);
        CSVManager::exportComparisonResults("test_comp_export.csv", compResults);
        std::ifstream in("test_comp_export.csv");
        ASSERT_EQ(in.is_open(), true, "CSV comparison export created");
        std::string firstLine;
        std::getline(in, firstLine);
        ASSERT_EQ(firstLine.find("# SchedOS Algorithm Comparison") != std::string::npos, true, "CSV comparison header correct");
    }
}

void testRepeatedSimulations() {
    auto procs = makeStandard3();
    FCFS fcfs;
    fcfs.schedule(procs);

    int ct1 = procs[0].completionTime;
    int ct2 = procs[1].completionTime;
    int ct3 = procs[2].completionTime;

    // Run a second time
    fcfs.schedule(procs);
    ASSERT_EQ(procs[0].completionTime, ct1, "Repeated simulation P1 CT identical");
    ASSERT_EQ(procs[1].completionTime, ct2, "Repeated simulation P2 CT identical");
    ASSERT_EQ(procs[2].completionTime, ct3, "Repeated simulation P3 CT identical");

    // Run third time with RoundRobin
    RoundRobin rr(2);
    rr.schedule(procs);
    int rrCt1 = procs[0].completionTime;
    rr.schedule(procs);
    ASSERT_EQ(procs[0].completionTime, rrCt1, "Repeated RR simulation P1 CT identical");
}

void testContextSwitchOrderChange() {
    // Scenario:
    // P1 arrives at 0, burst = 2
    // P2 arrives at 1, burst = 5
    // P3 arrives at 3, burst = 1
    //
    // WITHOUT context switch (CS=0):
    // t=0-2: P1 runs, finishes at 2.
    // At t=2: Only P2 has arrived. In SJF, P2 is the only option, so P2 runs t=2 to 7.
    // At t=7: P3 runs, finishes at 8.
    // Order: P1 (CT=2), P2 (CT=7), P3 (CT=8).
    std::vector<Process> procsNoCS = {
        Process(1, 0, 2),
        Process(2, 1, 5),
        Process(3, 3, 1)
    };
    SJF sjfNoCS;
    sjfNoCS.setContextSwitchCost(0);
    sjfNoCS.schedule(procsNoCS);
    ASSERT_EQ(procsNoCS[0].completionTime, 2, "No-CS P1 CT");
    ASSERT_EQ(procsNoCS[1].completionTime, 7, "No-CS P2 CT");
    ASSERT_EQ(procsNoCS[2].completionTime, 8, "No-CS P3 CT");

    // WITH context switch (CS=2):
    // t=0-2: P1 runs, finishes at 2.
    // t=2-4: Context switch interval [2-4].
    // At t=3: P3 arrives!
    // At t=4: Context switch completes. Candidates: P2 (burst=5) and P3 (burst=1).
    // SJF chooses P3 because burst 1 < 5!
    // t=4-5: P3 runs, finishes at 5!
    // t=5-7: Context switch to P2 [5-7].
    // t=7-12: P2 runs, finishes at 12!
    // Execution order changed from P1->P2->P3 to P1->P3->P2!
    std::vector<Process> procsWithCS = {
        Process(1, 0, 2),
        Process(2, 1, 5),
        Process(3, 3, 1)
    };
    SJF sjfWithCS;
    sjfWithCS.setContextSwitchCost(2);
    sjfWithCS.schedule(procsWithCS);
    ASSERT_EQ(procsWithCS[0].completionTime, 2, "With-CS P1 CT");
    ASSERT_EQ(procsWithCS[2].completionTime, 5, "With-CS P3 runs BEFORE P2 and finishes at 5");
    ASSERT_EQ(procsWithCS[1].completionTime, 12, "With-CS P2 finishes last at 12");
    ASSERT_EQ(sjfWithCS.contextSwitchCount(), 2, "With-CS 2 context switches charged");
}

void testTaskQueue() {
    // 1. Basic push, size, pop
    TaskQueue queue;
    ASSERT_EQ(queue.empty(), true, "TQ empty initially");
    ASSERT_EQ(static_cast<int>(queue.size()), 0, "TQ size 0 initially");

    int runCount = 0;
    queue.push([&runCount]() { runCount += 10; });
    queue.push([&runCount]() { runCount += 20; });
    ASSERT_EQ(queue.empty(), false, "TQ not empty after pushes");
    ASSERT_EQ(static_cast<int>(queue.size()), 2, "TQ size 2");

    Task t;
    bool popped1 = queue.pop(t);
    ASSERT_EQ(popped1, true, "TQ popped1 success");
    t();
    ASSERT_EQ(runCount, 10, "TQ task 1 executed");

    bool popped2 = queue.tryPop(t);
    ASSERT_EQ(popped2, true, "TQ tryPop success");
    t();
    ASSERT_EQ(runCount, 30, "TQ task 2 executed");

    bool popped3 = queue.tryPop(t);
    ASSERT_EQ(popped3, false, "TQ tryPop empty false");

    // 2. Stop wakes threads
    queue.stop();
    ASSERT_EQ(queue.isStopped(), true, "TQ isStopped true");
    bool poppedAfterStop = queue.pop(t);
    ASSERT_EQ(poppedAfterStop, false, "TQ pop after stop returns false");

    // 3. Multi-threaded Producer-Consumer
    TaskQueue mtQueue;
    std::atomic<int> processedSum{0};
    const int tasksPerProducer = 100;
    const int numProducers = 3;
    const int numConsumers = 3;

    std::vector<std::thread> consumers;
    for (int i = 0; i < numConsumers; ++i) {
        consumers.emplace_back([&mtQueue, &processedSum]() {
            Task task;
            while (mtQueue.pop(task)) {
                task();
            }
        });
    }

    std::vector<std::thread> producers;
    for (int p = 0; p < numProducers; ++p) {
        producers.emplace_back([&mtQueue, &processedSum, tasksPerProducer]() {
            for (int k = 0; k < tasksPerProducer; ++k) {
                mtQueue.push([&processedSum]() {
                    processedSum.fetch_add(1, std::memory_order_relaxed);
                });
            }
        });
    }

    for (auto& pt : producers) pt.join();
    // Wait until all tasks processed, then stop queue
    while (processedSum.load() < numProducers * tasksPerProducer) {
        std::this_thread::yield();
    }
    mtQueue.stop();
    for (auto& ct : consumers) ct.join();

    ASSERT_EQ(processedSum.load(), numProducers * tasksPerProducer, "TQ MT sum matches");
}

void testThreadPool() {
    // 1. Worker count configuration
    {
        ThreadPool pool(4);
        ASSERT_EQ(pool.workerCount(), static_cast<size_t>(4), "TP 4 workers");
        ASSERT_EQ(pool.isShutdown(), false, "TP not shut down initially");
    }

    // 2. Submit with std::future return
    {
        ThreadPool pool(3);
        auto fut1 = pool.submit([](int a, int b) { return a * b; }, 6, 7);
        auto fut2 = pool.submit([](const std::string& s) { return s + " World"; }, "Hello");

        ASSERT_EQ(fut1.get(), 42, "TP future multiply 42");
        ASSERT_EQ(fut2.get(), std::string("Hello World"), "TP future string");
    }

    // 3. submitVoid and waitForTasks()
    {
        ThreadPool pool(4);
        std::atomic<int> counter{0};
        const int numTasks = 50;

        for (int i = 0; i < numTasks; ++i) {
            pool.submitVoid([&counter]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                counter.fetch_add(1, std::memory_order_relaxed);
            });
        }

        pool.waitForTasks();
        ASSERT_EQ(counter.load(), numTasks, "TP waitForTasks all completed");
        ASSERT_EQ(pool.completedTaskCount(), static_cast<size_t>(numTasks), "TP completedTaskCount matches");
    }

    // 4. Exception resilience: task throws, pool worker survives
    {
        ThreadPool pool(2);
        auto futFail = pool.submit([]() -> int {
            throw std::runtime_error("Task deliberate exception");
        });

        bool caught = false;
        try {
            futFail.get();
        } catch (const std::runtime_error&) {
            caught = true;
        }
        ASSERT_EQ(caught, true, "TP future caught task exception");

        // Subsequent task still works!
        auto futSuccess = pool.submit([]() { return 100; });
        ASSERT_EQ(futSuccess.get(), 100, "TP worker survived exception");
    }

    // 5. Repeated creation and destruction without hang
    for (int rep = 0; rep < 5; ++rep) {
        ThreadPool pool(2);
        pool.submitVoid([]() {});
    }
}

void testBenchmarkEngine() {
    // 1. Task determinism
    uint64_t val1 = BenchmarkEngine::executeTask(42, 100);
    uint64_t val2 = BenchmarkEngine::executeTask(42, 100);
    ASSERT_EQ(val1, val2, "Benchmark task deterministic");

    // 2. Run small benchmark
    auto res = BenchmarkEngine::run(20, 2, 500, 1);
    ASSERT_EQ(res.numTasks, static_cast<size_t>(20), "Benchmark task count matches");
    ASSERT_EQ(res.resultsMatch, true, "Benchmark seq and par checksums match");
    ASSERT_EQ(res.sequentialChecksum, res.parallelChecksum, "Benchmark checksum equality");
    ASSERT_EQ(res.speedup > 0.0, true, "Benchmark positive speedup");

    // 3. Print report check
    std::ostringstream oss;
    BenchmarkEngine::printReport(res, oss, false);
    ASSERT_EQ(!oss.str().empty(), true, "Benchmark report non-empty");
    ASSERT_EQ(oss.str().find("CONCURRENCY & MULTITHREADING PERFORMANCE BENCHMARK") != std::string::npos, true, "Benchmark header found");
}

void testSchedulingFairness() {
    // Process set with known waiting times:
    // P1: arr=0, burst=4 -> CT=4, TAT=4, WT=0
    // P2: arr=0, burst=4 -> CT=8, TAT=8, WT=4
    // P3: arr=0, burst=4 -> CT=12, TAT=12, WT=8
    std::vector<Process> procs = {
        Process(1, 0, 4),
        Process(2, 0, 4),
        Process(3, 0, 4)
    };
    FCFS fcfs;
    fcfs.schedule(procs);

    auto m = SchedulingMetrics::compute(procs, fcfs.ganttChart());
    ASSERT_EQ(m.minWaitingTime, 0, "Fairness min WT");
    ASSERT_EQ(m.maxWaitingTime, 8, "Fairness max WT");
    ASSERT_NEAR(m.avgWaitingTime, 4.0, 0.001, "Fairness avg WT");
    // Variance = ((0-4)^2 + (4-4)^2 + (8-4)^2) / 3 = (16 + 0 + 16) / 3 = 32/3 = 10.6667
    ASSERT_NEAR(m.waitingTimeVariance, 32.0 / 3.0, 0.01, "Fairness WT variance");
    ASSERT_NEAR(m.waitingTimeStdDev, std::sqrt(32.0 / 3.0), 0.01, "Fairness WT std dev");

    // Zero variance case: single process
    std::vector<Process> single = { Process(1, 0, 5) };
    fcfs.schedule(single);
    auto mSingle = SchedulingMetrics::compute(single, fcfs.ganttChart());
    ASSERT_EQ(mSingle.minWaitingTime, 0, "Single proc min WT");
    ASSERT_EQ(mSingle.maxWaitingTime, 0, "Single proc max WT");
    ASSERT_NEAR(mSingle.waitingTimeVariance, 0.0, 0.001, "Single proc WT variance zero");
    ASSERT_NEAR(mSingle.waitingTimeStdDev, 0.0, 0.001, "Single proc WT std dev zero");
}

void testPriorityAging() {
    // Without aging:
    // P1: arr=0, burst=6, prio=1
    // P2: arr=1, burst=4, prio=5 (low priority)
    // P3: arr=5, burst=2, prio=3 (medium priority)
    // At t=6 (P1 ends): P2 (prio 5) vs P3 (prio 3). P3 runs first without aging!
    {
        std::vector<Process> procsNoAging = {
            Process(1, 0, 6, 1),
            Process(2, 1, 4, 5),
            Process(3, 5, 2, 3)
        };
        PriorityScheduling ps;
        ps.setAgingInterval(0); // Disabled
        ps.schedule(procsNoAging);

        // P3 should complete before P2
        ASSERT_EQ(procsNoAging[0].completionTime, 6,  "No-aging P1 CT");
        ASSERT_EQ(procsNoAging[2].completionTime, 8,  "No-aging P3 CT (runs before P2)");
        ASSERT_EQ(procsNoAging[1].completionTime, 12, "No-aging P2 CT (starved until P3 finishes)");
    }

    // With aging: aging interval = 1
    // At t=6:
    // P2 has waited (6 - 1) = 5 time units -> effective prio = max(0, 5 - 5) = 0!
    // P3 has waited (6 - 5) = 1 time unit  -> effective prio = max(0, 3 - 1) = 2.
    // P2 has higher effective priority (0 < 2), so P2 runs FIRST!
    {
        std::vector<Process> procsAging = {
            Process(1, 0, 6, 1),
            Process(2, 1, 4, 5),
            Process(3, 5, 2, 3)
        };
        PriorityScheduling ps;
        ps.setAgingInterval(1);
        ASSERT_EQ(ps.agingInterval(), 1, "Aging interval getter");
        ASSERT_EQ(ps.name(), "Priority Scheduling (Non-Preemptive with Aging)", "Aging scheduler name");
        ps.schedule(procsAging);

        // P2 should complete before P3!
        ASSERT_EQ(procsAging[0].completionTime, 6,  "Aging P1 CT");
        ASSERT_EQ(procsAging[1].completionTime, 10, "Aging P2 CT (promoted by aging, runs before P3)");
        ASSERT_EQ(procsAging[2].completionTime, 12, "Aging P3 CT (runs after promoted P2)");
    }
}

void testStressWorkloads() {
    // Test 100, 1,000, and 10,000 processes with FCFS and SJF
    const std::vector<int> counts = {100, 1000, 10000};

    for (int count : counts) {
        std::vector<Process> procs;
        procs.reserve(count);
        for (int i = 0; i < count; ++i) {
            // Predictable varied arrival times and bursts
            int arrival = (i % 50);
            int burst = (i % 20) + 1;
            int priority = (i % 10) + 1;
            procs.emplace_back(i + 1, arrival, burst, priority);
        }

        FCFS fcfs;
        fcfs.schedule(procs);

        // Verify all completed
        bool allCompleted = true;
        for (const auto& p : procs) {
            if (p.completionTime <= 0 || p.turnaroundTime <= 0 || p.waitingTime < 0) {
                allCompleted = false;
                break;
            }
        }
        ASSERT_EQ(allCompleted, true, "Stress FCFS all " + std::to_string(count) + " processes valid");

        auto metrics = SchedulingMetrics::compute(procs, fcfs.ganttChart());
        ASSERT_EQ(metrics.throughput > 0.0, true, "Stress FCFS " + std::to_string(count) + " positive throughput");
        ASSERT_EQ(metrics.avgTurnaroundTime > 0.0, true, "Stress FCFS " + std::to_string(count) + " positive avg TAT");
    }
}

void testThreadPoolStress() {
    // Multi-producer stress test: 4 producer threads, each submitting 50 tasks to a 4-worker pool
    const int numProducers = 4;
    const int tasksPerProducer = 50;
    const int totalTasks = numProducers * tasksPerProducer;

    ThreadPool pool(4);
    std::atomic<int> completedTasks{0};
    std::vector<std::future<int>> futures;
    std::mutex futuresMutex;

    std::vector<std::thread> producers;
    for (int p = 0; p < numProducers; ++p) {
        producers.emplace_back([&, p]() {
            for (int i = 0; i < tasksPerProducer; ++i) {
                int taskId = p * tasksPerProducer + i;
                auto fut = pool.submit([&completedTasks, taskId]() {
                    // Small computation
                    uint64_t sum = 0;
                    for (int k = 0; k < 1000; ++k) sum += k;
                    completedTasks.fetch_add(1, std::memory_order_relaxed);
                    return taskId;
                });

                std::lock_guard<std::mutex> lock(futuresMutex);
                futures.push_back(std::move(fut));
            }
        });
    }

    for (auto& t : producers) {
        t.join();
    }

    // Wait for all futures and verify outputs
    int receivedCount = 0;
    for (auto& fut : futures) {
        int id = fut.get();
        if (id >= 0 && id < totalTasks) {
            receivedCount++;
        }
    }

    ASSERT_EQ(receivedCount, totalTasks, "TP stress all futures resolved");
    ASSERT_EQ(completedTasks.load(), totalTasks, "TP stress atomic counter matches total");
}

void testIndependent_FCFS_ContextSwitchIdle() {
    // Independently calculated:
    // P1: arr 1, burst 3 -> CT 4, WT 0, TAT 3, RT 0
    // P2: arr 2, burst 2 -> CT 7, WT 3, TAT 5, RT 3 (starts after CS at t=4..5)
    // P3: arr 8, burst 4 -> CT 12, WT 0, TAT 4, RT 0 (starts after idle at t=7..8)
    std::vector<Process> procs = {
        Process(1, 1, 3),
        Process(2, 2, 2),
        Process(3, 8, 4)
    };
    FCFS fcfs;
    fcfs.setContextSwitchCost(1);
    fcfs.schedule(procs);

    ASSERT_EQ(procs[0].completionTime, 4,  "Indep FCFS P1 CT");
    ASSERT_EQ(procs[0].waitingTime,    0,  "Indep FCFS P1 WT");
    ASSERT_EQ(procs[0].turnaroundTime, 3,  "Indep FCFS P1 TAT");
    ASSERT_EQ(procs[0].responseTime,   0,  "Indep FCFS P1 RT");

    ASSERT_EQ(procs[1].completionTime, 7,  "Indep FCFS P2 CT");
    ASSERT_EQ(procs[1].waitingTime,    3,  "Indep FCFS P2 WT");
    ASSERT_EQ(procs[1].turnaroundTime, 5,  "Indep FCFS P2 TAT");
    ASSERT_EQ(procs[1].responseTime,   3,  "Indep FCFS P2 RT");

    ASSERT_EQ(procs[2].completionTime, 12, "Indep FCFS P3 CT");
    ASSERT_EQ(procs[2].waitingTime,    0,  "Indep FCFS P3 WT");
    ASSERT_EQ(procs[2].turnaroundTime, 4,  "Indep FCFS P3 TAT");
    ASSERT_EQ(procs[2].responseTime,   0,  "Indep FCFS P3 RT");

    ASSERT_EQ(fcfs.contextSwitchCount(), 1, "Indep FCFS CS count");
    ASSERT_EQ(fcfs.totalContextSwitchOverhead(), 1, "Indep FCFS CS overhead");

    auto m = SchedulingMetrics::compute(procs, fcfs.ganttChart(), 1, 1);
    ASSERT_NEAR(m.avgWaitingTime, 1.0, 0.001, "Indep FCFS avg WT");
    ASSERT_NEAR(m.avgTurnaroundTime, 4.0, 0.001, "Indep FCFS avg TAT");
    ASSERT_NEAR(m.productiveCpuUtilization, 75.0, 0.01, "Indep FCFS prod CPU");
    ASSERT_NEAR(m.totalCpuOccupancy, (10.0 / 12.0) * 100.0, 0.01, "Indep FCFS total occupancy");
}

void testIndependent_SJF_SimultaneousCS() {
    // Independently calculated:
    // P1: arr 0, burst 5
    // P2: arr 0, burst 2
    // P3: arr 0, burst 4
    // CS = 1
    // Order: P2 [0..2] -> CS [2..3] -> P3 [3..7] -> CS [7..8] -> P1 [8..13]
    std::vector<Process> procs = {
        Process(1, 0, 5),
        Process(2, 0, 2),
        Process(3, 0, 4)
    };
    SJF sjf;
    sjf.setContextSwitchCost(1);
    sjf.schedule(procs);

    ASSERT_EQ(procs[1].completionTime, 2,  "Indep SJF P2 CT");
    ASSERT_EQ(procs[1].waitingTime,    0,  "Indep SJF P2 WT");
    ASSERT_EQ(procs[1].responseTime,   0,  "Indep SJF P2 RT");

    ASSERT_EQ(procs[2].completionTime, 7,  "Indep SJF P3 CT");
    ASSERT_EQ(procs[2].waitingTime,    3,  "Indep SJF P3 WT");
    ASSERT_EQ(procs[2].responseTime,   3,  "Indep SJF P3 RT");

    ASSERT_EQ(procs[0].completionTime, 13, "Indep SJF P1 CT");
    ASSERT_EQ(procs[0].waitingTime,    8,  "Indep SJF P1 WT");
    ASSERT_EQ(procs[0].responseTime,   8,  "Indep SJF P1 RT");

    ASSERT_EQ(sjf.contextSwitchCount(), 2, "Indep SJF CS count");
    ASSERT_EQ(sjf.totalContextSwitchOverhead(), 2, "Indep SJF CS overhead");

    auto m = SchedulingMetrics::compute(procs, sjf.ganttChart(), 2, 2);
    ASSERT_NEAR(m.avgWaitingTime, 11.0 / 3.0, 0.01, "Indep SJF avg WT");
    ASSERT_NEAR(m.avgTurnaroundTime, 22.0 / 3.0, 0.01, "Indep SJF avg TAT");
    ASSERT_NEAR(m.productiveCpuUtilization, (11.0 / 13.0) * 100.0, 0.01, "Indep SJF prod CPU");
    ASSERT_NEAR(m.totalCpuOccupancy, 100.0, 0.01, "Indep SJF total occupancy");
}

void testIndependent_SRTF_MultiPreemption() {
    // Independently calculated:
    // P1: arr 0, burst 8
    // P2: arr 1, burst 4
    // P3: arr 2, burst 2
    // P4: arr 3, burst 1
    // CS = 0
    // Execution: P1 [0..1] -> P2 [1..2] -> P3 [2..4] -> P4 [4..5] -> P2 [5..8] -> P1 [8..15]
    std::vector<Process> procs = {
        Process(1, 0, 8),
        Process(2, 1, 4),
        Process(3, 2, 2),
        Process(4, 3, 1)
    };
    SRTF srtf;
    srtf.schedule(procs);

    ASSERT_EQ(procs[2].completionTime, 4,  "Indep SRTF P3 CT");
    ASSERT_EQ(procs[2].waitingTime,    0,  "Indep SRTF P3 WT");
    ASSERT_EQ(procs[2].responseTime,   0,  "Indep SRTF P3 RT");

    ASSERT_EQ(procs[3].completionTime, 5,  "Indep SRTF P4 CT");
    ASSERT_EQ(procs[3].waitingTime,    1,  "Indep SRTF P4 WT");
    ASSERT_EQ(procs[3].responseTime,   1,  "Indep SRTF P4 RT");

    ASSERT_EQ(procs[1].completionTime, 8,  "Indep SRTF P2 CT");
    ASSERT_EQ(procs[1].waitingTime,    3,  "Indep SRTF P2 WT");
    ASSERT_EQ(procs[1].responseTime,   0,  "Indep SRTF P2 RT");

    ASSERT_EQ(procs[0].completionTime, 15, "Indep SRTF P1 CT");
    ASSERT_EQ(procs[0].waitingTime,    7,  "Indep SRTF P1 WT");
    ASSERT_EQ(procs[0].responseTime,   0,  "Indep SRTF P1 RT");

    auto m = SchedulingMetrics::compute(procs, srtf.ganttChart());
    ASSERT_NEAR(m.avgWaitingTime, 2.75, 0.001, "Indep SRTF avg WT");
    ASSERT_NEAR(m.avgTurnaroundTime, 6.50, 0.001, "Indep SRTF avg TAT");
    ASSERT_NEAR(m.avgResponseTime, 0.25, 0.001, "Indep SRTF avg RT");
}

void testIndependent_RR_PreemptionCS() {
    // Independently calculated:
    // P1: arr 0, burst 5
    // P2: arr 1, burst 3
    // Quantum = 2, CS = 1
    // Timeline: P1[0..2] -> CS[2..3] -> P2[3..5] -> CS[5..6] -> P1[6..8] -> CS[8..9] -> P2[9..10] -> CS[10..11] -> P1[11..12]
    std::vector<Process> procs = {
        Process(1, 0, 5),
        Process(2, 1, 3)
    };
    RoundRobin rr(2);
    rr.setContextSwitchCost(1);
    rr.schedule(procs);

    ASSERT_EQ(procs[1].completionTime, 10, "Indep RR P2 CT");
    ASSERT_EQ(procs[1].turnaroundTime, 9,  "Indep RR P2 TAT");
    ASSERT_EQ(procs[1].waitingTime,    6,  "Indep RR P2 WT");
    ASSERT_EQ(procs[1].responseTime,   2,  "Indep RR P2 RT");

    ASSERT_EQ(procs[0].completionTime, 12, "Indep RR P1 CT");
    ASSERT_EQ(procs[0].turnaroundTime, 12, "Indep RR P1 TAT");
    ASSERT_EQ(procs[0].waitingTime,    7,  "Indep RR P1 WT");
    ASSERT_EQ(procs[0].responseTime,   0,  "Indep RR P1 RT");

    ASSERT_EQ(rr.contextSwitchCount(), 4, "Indep RR CS count");
    ASSERT_EQ(rr.totalContextSwitchOverhead(), 4, "Indep RR CS overhead");

    auto m = SchedulingMetrics::compute(procs, rr.ganttChart(), 4, 4);
    ASSERT_NEAR(m.productiveCpuUtilization, (8.0 / 12.0) * 100.0, 0.01, "Indep RR prod CPU");
    ASSERT_NEAR(m.totalCpuOccupancy, 100.0, 0.01, "Indep RR total occupancy");
}

void testIndependent_Priority_AgingStarvation() {
    // Independently calculated:
    // P1: arr 0, burst 8, prio 1
    // P2: arr 1, burst 2, prio 5
    // P3: arr 7, burst 2, prio 2
    // CS = 0
    // Without aging: at t=8, P3(prio 2) precedes P2(prio 5). P3 CT=10, P2 CT=12.
    // With aging (interval=2): at t=8, P2 effective prio = max(0, 5 - 7/2) = 2.
    // P3 effective prio = max(0, 2 - 1/2) = 2. Tie break by arrival: P2 arr 1 vs P3 arr 7 -> P2 runs first!
    // P2 CT=10, P3 CT=12.
    {
        std::vector<Process> procs = {
            Process(1, 0, 8, 1),
            Process(2, 1, 2, 5),
            Process(3, 7, 2, 2)
        };
        PriorityScheduling ps;
        ps.setAgingInterval(0);
        ps.schedule(procs);

        ASSERT_EQ(procs[0].completionTime, 8,  "Indep Prio NoAging P1 CT");
        ASSERT_EQ(procs[2].completionTime, 10, "Indep Prio NoAging P3 CT (earlier)");
        ASSERT_EQ(procs[1].completionTime, 12, "Indep Prio NoAging P2 CT (later)");
    }
    {
        std::vector<Process> procs = {
            Process(1, 0, 8, 1),
            Process(2, 1, 2, 5),
            Process(3, 7, 2, 2)
        };
        PriorityScheduling ps;
        ps.setAgingInterval(2);
        ps.schedule(procs);

        ASSERT_EQ(procs[0].completionTime, 8,  "Indep Prio Aging P1 CT");
        ASSERT_EQ(procs[1].completionTime, 10, "Indep Prio Aging P2 CT (promoted by aging)");
        ASSERT_EQ(procs[2].completionTime, 12, "Indep Prio Aging P3 CT (runs after aged P2)");
    }
}

int main() {
    std::cout << "\n========================================\n";
    std::cout <<   "   SchedOS — Scheduling Test Suite     \n";
    std::cout <<   "========================================\n\n";

    runSection("FCFS Tests", [] {
        testFCFS_basic();
        testFCFS_simultaneousArrival();
        testFCFS_idlePeriod();
        testFCFS_singleProcess();
    });

    runSection("SJF Tests", [] {
        testSJF_basic();
        testSJF_tieBreaking();
    });

    runSection("SRTF Tests", [] {
        testSRTF_basic();
        testSRTF_noPreemption();
    });

    runSection("Round Robin Tests", [] {
        testRR_basic();
        testRR_quantum1();
        testRR_quantumLargerThanBurst();
        testRR_invalidQuantum();
    });

    runSection("Priority Scheduling Tests", [] {
        testPriority_basic();
        testPriority_samePriority();
    });

    runSection("Metrics Tests", [] {
        testMetrics_basic();
        testMetrics_withIdle();
    });

    runSection("Edge Case Tests", [] {
        testInvalidProcess();
        testEmptySchedule();
        testReadProcesses();
    });

    runSection("ProcessManager Tests", [] {
        testProcessManager();
    });

    runSection("GanttChart Tests", [] {
        testGanttChart();
    });

    runSection("Context Switching Tests", [] {
        testContextSwitching();
    });

    runSection("ComparisonEngine Tests", [] {
        testComparisonEngine();
    });

    runSection("CSVManager Tests", [] {
        testCSVManager();
    });

    runSection("Advanced Verification Tests", [] {
        testRepeatedSimulations();
        testContextSwitchOrderChange();
    });

    runSection("TaskQueue Tests", [] {
        testTaskQueue();
    });

    runSection("ThreadPool Tests", [] {
        testThreadPool();
    });

    runSection("BenchmarkEngine Tests", [] {
        testBenchmarkEngine();
    });

    runSection("Fairness & Aging Tests", [] {
        testSchedulingFairness();
        testPriorityAging();
    });

    runSection("Stress & Concurrency Scale Tests", [] {
        testStressWorkloads();
        testThreadPoolStress();
    });

    runSection("Independent Algorithm Verification Tests", [] {
        testIndependent_FCFS_ContextSwitchIdle();
        testIndependent_SJF_SimultaneousCS();
        testIndependent_SRTF_MultiPreemption();
        testIndependent_RR_PreemptionCS();
        testIndependent_Priority_AgingStarvation();
    });

    std::cout << "\n========================================\n";
    std::cout << "  Results: " << passedTests << " passed, "
              << failedTests << " failed, "
              << totalTests << " total\n";
    std::cout << "========================================\n\n";

    return failedTests > 0 ? 1 : 0;
}
