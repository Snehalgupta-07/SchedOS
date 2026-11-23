#pragma once

#include "schedos/Process.h"
#include "schedos/Metrics.h"
#include "schedos/ComparisonEngine.h"
#include <string>
#include <vector>

namespace SchedOS {

class CSVManager {
public:
    /// Imports processes from a CSV file.
    /// Validates CSV structure, missing values, non-numbers, negative arrivals, non-positive bursts, duplicate PIDs.
    /// Throws std::runtime_error or std::invalid_argument with descriptive messages on failure.
    static std::vector<Process> importProcesses(const std::string& filepath);

    /// Exports process simulation results and summary metrics to a CSV file.
    static void exportScheduleResults(const std::string& filepath,
                                      const std::string& algorithmName,
                                      const std::vector<Process>& processes,
                                      const SchedulingMetrics& metrics);

    /// Exports algorithm comparison benchmark results to a CSV file.
    static void exportComparisonResults(const std::string& filepath,
                                        const std::vector<ComparisonResult>& results);
};

} // namespace SchedOS
