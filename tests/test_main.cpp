// SchedOS — Lightweight test harness (no external dependencies)
// Tests all five scheduling algorithms with known expected results.

#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <functional>

#include "schedos/Process.h"
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

    std::cout << "\n========================================\n";
    std::cout << "  Results: " << passedTests << " passed, "
              << failedTests << " failed, "
              << totalTests << " total\n";
    std::cout << "========================================\n\n";

    return failedTests > 0 ? 1 : 0;
}
