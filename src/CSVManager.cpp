#include "schedos/CSVManager.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <set>
#include <algorithm>

namespace SchedOS {

static std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, (last - first + 1));
}

std::vector<Process> CSVManager::importProcesses(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open CSV file: " + filepath);
    }

    std::vector<Process> processes;
    std::set<int> seenPids;
    std::string line;
    int lineNum = 0;
    bool hasData = false;

    while (std::getline(file, line)) {
        ++lineNum;
        std::string trimmedLine = trim(line);
        if (trimmedLine.empty() || trimmedLine[0] == '#') {
            continue; // Ignore blank lines and comments
        }

        // Header detection on first data line
        if (!hasData) {
            std::string lowerLine = trimmedLine;
            std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);
            if (lowerLine.find("pid") != std::string::npos || lowerLine.find("process") != std::string::npos) {
                continue; // Skip header row
            }
        }

        hasData = true;

        std::stringstream ss(trimmedLine);
        std::string cell;
        std::vector<std::string> row;

        while (std::getline(ss, cell, ',')) {
            row.push_back(trim(cell));
        }

        if (row.size() < 3) {
            throw std::invalid_argument("CSV line " + std::to_string(lineNum) +
                                       ": Insufficient columns (expected at least PID, Arrival, Burst). Got: " + trimmedLine);
        }

        // Parse PID
        int pid;
        try {
            size_t idx;
            pid = std::stoi(row[0], &idx);
            if (idx != row[0].length()) throw std::invalid_argument("");
        } catch (...) {
            throw std::invalid_argument("CSV line " + std::to_string(lineNum) +
                                       ": Invalid non-integer PID '" + row[0] + "'");
        }

        if (pid < 0) {
            throw std::invalid_argument("CSV line " + std::to_string(lineNum) +
                                       ": Process ID cannot be negative (got " + std::to_string(pid) + ")");
        }

        if (seenPids.count(pid) > 0) {
            throw std::invalid_argument("CSV line " + std::to_string(lineNum) +
                                       ": Duplicate Process ID " + std::to_string(pid) + " detected.");
        }

        // Parse Arrival Time
        int arrival;
        try {
            size_t idx;
            arrival = std::stoi(row[1], &idx);
            if (idx != row[1].length()) throw std::invalid_argument("");
        } catch (...) {
            throw std::invalid_argument("CSV line " + std::to_string(lineNum) +
                                       ": Invalid non-integer arrival time '" + row[1] + "'");
        }

        if (arrival < 0) {
            throw std::invalid_argument("CSV line " + std::to_string(lineNum) +
                                       ": Arrival time cannot be negative (got " + std::to_string(arrival) + ")");
        }

        // Parse Burst Time
        int burst;
        try {
            size_t idx;
            burst = std::stoi(row[2], &idx);
            if (idx != row[2].length()) throw std::invalid_argument("");
        } catch (...) {
            throw std::invalid_argument("CSV line " + std::to_string(lineNum) +
                                       ": Invalid non-integer burst time '" + row[2] + "'");
        }

        if (burst <= 0) {
            throw std::invalid_argument("CSV line " + std::to_string(lineNum) +
                                       ": Burst time must be positive (> 0, got " + std::to_string(burst) + ")");
        }

        // Parse Priority (optional, defaults to 0)
        int priority = 0;
        if (row.size() >= 4 && !row[3].empty()) {
            try {
                size_t idx;
                priority = std::stoi(row[3], &idx);
                if (idx != row[3].length()) throw std::invalid_argument("");
            } catch (...) {
                throw std::invalid_argument("CSV line " + std::to_string(lineNum) +
                                           ": Invalid non-integer priority '" + row[3] + "'");
            }
            if (priority < 0) {
                throw std::invalid_argument("CSV line " + std::to_string(lineNum) +
                                           ": Priority cannot be negative (got " + std::to_string(priority) + ")");
            }
        }

        seenPids.insert(pid);
        processes.emplace_back(pid, arrival, burst, priority);
    }

    if (!hasData || processes.empty()) {
        throw std::runtime_error("CSV file contains no valid process data: " + filepath);
    }

    return processes;
}

void CSVManager::exportScheduleResults(const std::string& filepath,
                                       const std::string& algorithmName,
                                       const std::vector<Process>& processes,
                                       const SchedulingMetrics& metrics) {
    std::ofstream out(filepath);
    if (!out.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filepath);
    }

    out << "# SchedOS Simulation Results\n";
    out << "# Algorithm," << algorithmName << "\n";
    out << "# TotalSimulationTime," << metrics.totalSimulationDuration << "\n";
    out << "# TotalBusyTime," << metrics.totalBusyTime << "\n";
    out << "# TotalIdleTime," << metrics.totalIdleTime << "\n";
    out << "# ContextSwitchCount," << metrics.contextSwitchCount << "\n";
    out << "# ContextSwitchOverhead," << metrics.totalContextSwitchOverhead << "\n";
    out << "# AverageWaitingTime," << metrics.avgWaitingTime << "\n";
    out << "# AverageTurnaroundTime," << metrics.avgTurnaroundTime << "\n";
    out << "# AverageResponseTime," << metrics.avgResponseTime << "\n";
    out << "# ProductiveCPUUtilizationPercent," << metrics.productiveCpuUtilization << "\n";
    out << "# TotalCPUOccupancyPercent," << metrics.totalCpuOccupancy << "\n";
    out << "# Throughput," << metrics.throughput << "\n";
    out << "\n";

    out << "PID,ArrivalTime,BurstTime,Priority,CompletionTime,TurnaroundTime,WaitingTime,ResponseTime\n";
    for (const auto& p : processes) {
        out << p.pid << ","
            << p.arrivalTime << ","
            << p.burstTime << ","
            << p.priority << ","
            << p.completionTime << ","
            << p.turnaroundTime << ","
            << p.waitingTime << ","
            << p.responseTime << "\n";
    }
}

void CSVManager::exportComparisonResults(const std::string& filepath,
                                         const std::vector<ComparisonResult>& results) {
    std::ofstream out(filepath);
    if (!out.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filepath);
    }

    out << "# SchedOS Algorithm Comparison Benchmark\n";
    out << "Algorithm,AvgWaitingTime,AvgTurnaroundTime,AvgResponseTime,ProductiveCPUUtilPercent,TotalCPUOccupancyPercent,Throughput,ContextSwitchCount,TotalExecutionTime\n";

    for (const auto& r : results) {
        out << "\"" << r.algorithmName << "\","
            << r.metrics.avgWaitingTime << ","
            << r.metrics.avgTurnaroundTime << ","
            << r.metrics.avgResponseTime << ","
            << r.metrics.productiveCpuUtilization << ","
            << r.metrics.totalCpuOccupancy << ","
            << r.metrics.throughput << ","
            << r.contextSwitchCount << ","
            << r.totalExecutionTime << "\n";
    }
}

} // namespace SchedOS
