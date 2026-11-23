#include "schedos/BenchmarkEngine.h"
#include "schedos/ThreadPool.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>

namespace SchedOS {

uint64_t BenchmarkEngine::executeTask(uint64_t taskId, int iterations) {
    // Deterministic CPU-bound computation: mix bit-shifts, multiplications, and prime tests
    volatile uint64_t acc = taskId + 0x9e3779b97f4a7c15ULL;
    for (int i = 0; i < iterations; ++i) {
        acc ^= (acc << 13);
        acc ^= (acc >> 7);
        acc ^= (acc << 17);
        acc = acc * 0xbf58476d1ce4e5b9ULL + static_cast<uint64_t>(i);
    }
    return acc;
}

BenchmarkResult BenchmarkEngine::run(size_t numTasks,
                                     size_t numThreads,
                                     int workIterations,
                                     int repetitions) {
    BenchmarkResult res;
    res.numTasks = numTasks;
    res.workIterations = workIterations;
    res.repetitions = std::max(1, repetitions);

    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) numThreads = 2;
    }
    res.numThreads = numThreads;

    // 1. Warm-up run (ensures CPU frequency scaling and thread cache warm-up)
    {
        ThreadPool warmPool(numThreads);
        for (size_t i = 0; i < std::min<size_t>(numTasks, 10); ++i) {
            warmPool.submitVoid([i, workIterations]() {
                executeTask(i, workIterations / 2);
            });
        }
        warmPool.waitForTasks();
    }

    std::vector<double> seqTimes;
    std::vector<double> parTimes;
    uint64_t lastSeqChecksum = 0;
    uint64_t lastParChecksum = 0;

    for (int rep = 0; rep < res.repetitions; ++rep) {
        // Sequential Execution
        std::vector<uint64_t> seqResults(numTasks, 0);
        auto tSeqStart = std::chrono::steady_clock::now();
        for (size_t i = 0; i < numTasks; ++i) {
            seqResults[i] = executeTask(i, workIterations);
        }
        auto tSeqEnd = std::chrono::steady_clock::now();
        double seqMs = std::chrono::duration<double, std::milli>(tSeqEnd - tSeqStart).count();
        seqTimes.push_back(seqMs);

        uint64_t seqCheck = 0;
        for (auto val : seqResults) seqCheck += val;
        lastSeqChecksum = seqCheck;

        // Parallel Execution
        std::vector<uint64_t> parResults(numTasks, 0);
        ThreadPool pool(numThreads);
        auto tParStart = std::chrono::steady_clock::now();

        std::vector<std::future<uint64_t>> futures;
        futures.reserve(numTasks);

        for (size_t i = 0; i < numTasks; ++i) {
            futures.push_back(pool.submit([i, workIterations]() {
                return executeTask(i, workIterations);
            }));
        }

        for (size_t i = 0; i < numTasks; ++i) {
            parResults[i] = futures[i].get();
        }
        auto tParEnd = std::chrono::steady_clock::now();
        double parMs = std::chrono::duration<double, std::milli>(tParEnd - tParStart).count();
        parTimes.push_back(parMs);

        uint64_t parCheck = 0;
        for (auto val : parResults) parCheck += val;
        lastParChecksum = parCheck;
    }

    // Compute Medians
    std::sort(seqTimes.begin(), seqTimes.end());
    std::sort(parTimes.begin(), parTimes.end());
    res.sequentialTimeMs = seqTimes[seqTimes.size() / 2];
    res.parallelTimeMs   = parTimes[parTimes.size() / 2];

    res.speedup = (res.parallelTimeMs > 0.0) ? (res.sequentialTimeMs / res.parallelTimeMs) : 1.0;
    res.sequentialThroughput = (res.sequentialTimeMs > 0.0) ? (numTasks / (res.sequentialTimeMs / 1000.0)) : 0.0;
    res.parallelThroughput   = (res.parallelTimeMs > 0.0)   ? (numTasks / (res.parallelTimeMs / 1000.0))   : 0.0;

    res.sequentialChecksum = lastSeqChecksum;
    res.parallelChecksum   = lastParChecksum;
    res.resultsMatch       = (lastSeqChecksum == lastParChecksum);

    return res;
}

void BenchmarkEngine::printReport(const BenchmarkResult& res,
                                  std::ostream& out,
                                  bool useColors) {
    const std::string green  = useColors ? "\033[32m\033[1m" : "";
    const std::string cyan   = useColors ? "\033[36m\033[1m" : "";
    const std::string yellow = useColors ? "\033[33m\033[1m" : "";
    const std::string red    = useColors ? "\033[31m\033[1m" : "";
    const std::string bold   = useColors ? "\033[1m" : "";
    const std::string reset  = useColors ? "\033[0m" : "";

    out << "\n" << cyan << "======================================================================\n";
    out << "            CONCURRENCY & MULTITHREADING PERFORMANCE BENCHMARK        \n";
    out << "======================================================================\n" << reset;

    out << std::fixed << std::setprecision(2);
    out << "  Workload Configuration:\n";
    out << "    Total CPU-Bound Tasks     : " << res.numTasks << "\n";
    out << "    Iterations Per Task       : " << res.workIterations << "\n";
    out << "    Worker Thread Count       : " << res.numThreads << "\n";
    out << "    Benchmark Repetitions     : " << res.repetitions << " (Median reported)\n\n";

    out << "  Execution Timing Results:\n";
    out << "    Sequential Execution Time : " << res.sequentialTimeMs << " ms\n";
    out << "    Multithreaded Time        : " << res.parallelTimeMs << " ms\n";
    out << "    Speedup Ratio             : " << (res.speedup >= 1.0 ? green : yellow)
        << res.speedup << "x" << reset << "\n\n";

    out << "  Throughput Analysis:\n";
    out << "    Sequential Throughput     : " << res.sequentialThroughput << " tasks/sec\n";
    out << "    Multithreaded Throughput  : " << green << res.parallelThroughput << " tasks/sec" << reset << "\n\n";

    out << "  Result Validation:\n";
    out << "    Sequential Checksum       : 0x" << std::hex << res.sequentialChecksum << std::dec << "\n";
    out << "    Parallel Checksum         : 0x" << std::hex << res.parallelChecksum << std::dec << "\n";
    out << "    Equivalence Check         : "
        << (res.resultsMatch ? (green + "PASS (Identical Results)" + reset) : (red + "FAIL (Checksum Mismatch)" + reset))
        << "\n\n";

    out << yellow << "  Engineering Note (Amdahl's Law & Concurrency Analysis):\n" << reset;
    out << "    - Theoretical max speedup is bounded by hardware core count (" << res.numThreads << " threads).\n";
    out << "    - Real-world speedup reflects thread synchronization, queue mutex contention,\n";
    out << "      and CPU memory bus bandwidth.\n";
    out << "    - Small workloads may experience thread management overhead; compute-heavy\n";
    out << "      tasks exhibit near-linear scaling up to physical core count.\n";
    out << cyan << "======================================================================\n\n" << reset;
}

} // namespace SchedOS
