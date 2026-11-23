#pragma once

#include "schedos/Process.h"
#include "schedos/Metrics.h"
#include <string>
#include <vector>
#include <iosfwd>

namespace SchedOS {

struct ComparisonResult {
    std::string algorithmName;
    std::string algorithmKey;
    SchedulingMetrics metrics;
    int contextSwitchCount = 0;
    int totalExecutionTime = 0;
};

class ComparisonEngine {
public:
    /// Runs all five scheduling algorithms on identical process dataset and returns their metrics.
    static std::vector<ComparisonResult> compareAll(const std::vector<Process>& processes,
                                                   int quantum = 2,
                                                   int csCost = 0);

    /// Formats and displays an aligned comparison table with best-performer highlights.
    static void printComparisonTable(const std::vector<ComparisonResult>& results,
                                    std::ostream& out,
                                    bool useColors = true);

    /// Prints a comprehensive operating system scheduling trade-off analysis.
    static void printTradeoffAnalysis(std::ostream& out,
                                      bool useColors = true);
};

} // namespace SchedOS
