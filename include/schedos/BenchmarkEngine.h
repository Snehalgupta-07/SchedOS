#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <cstdint>
#include <iosfwd>

namespace SchedOS {

struct BenchmarkResult {
    size_t numTasks = 0;
    size_t numThreads = 0;
    int workIterations = 0;
    int repetitions = 0;

    double sequentialTimeMs = 0.0;
    double parallelTimeMs = 0.0;
    double speedup = 0.0;

    double sequentialThroughput = 0.0; // tasks/sec
    double parallelThroughput   = 0.0; // tasks/sec

    bool resultsMatch = false;
    uint64_t sequentialChecksum = 0;
    uint64_t parallelChecksum = 0;
};

class BenchmarkEngine {
public:
    /// Executes a CPU-bound workload task. Returns a deterministic checksum.
    /// Uses volatile accumulators to prevent compiler dead-code elimination.
    static uint64_t executeTask(uint64_t taskId, int iterations);

    /// Runs a comparative benchmark between sequential and multithreaded execution.
    /// Runs warm-up rounds and takes the median of multiple repetitions for timing stability.
    static BenchmarkResult run(size_t numTasks = 100,
                               size_t numThreads = 0,
                               int workIterations = 25000,
                               int repetitions = 3);

    /// Prints a comprehensive benchmark performance report.
    static void printReport(const BenchmarkResult& res,
                            std::ostream& out,
                            bool useColors = true);
};

} // namespace SchedOS
