#include "schedos/GanttChart.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <map>

namespace SchedOS {

static std::string colorFor(int pid, bool useColors) {
    if (!useColors) return "";
    if (pid == -1) return "\033[90m";       // Gray for Idle
    if (pid == -2) return "\033[33m";       // Yellow for Context Switch
    // Color cycle for processes
    const char* colors[] = {
        "\033[36m", // Cyan
        "\033[32m", // Green
        "\033[35m", // Magenta
        "\033[34m", // Blue
        "\033[96m", // Bright Cyan
        "\033[92m", // Bright Green
        "\033[95m", // Bright Magenta
        "\033[93m"  // Bright Yellow
    };
    return colors[pid % 8];
}

static std::string resetCol(bool useColors) {
    return useColors ? "\033[0m" : "";
}

static std::string labelFor(int pid) {
    if (pid == -1) return "IDLE";
    if (pid == -2) return "CS";
    return "P" + std::to_string(pid);
}

std::vector<GanttEntry> GanttChart::mergeAdjacent(const std::vector<GanttEntry>& raw) {
    if (raw.empty()) return {};

    std::vector<GanttEntry> merged;
    merged.reserve(raw.size());

    for (const auto& entry : raw) {
        int pid = std::get<0>(entry);
        int start = std::get<1>(entry);
        int end = std::get<2>(entry);

        if (start == end) continue; // Skip zero-duration intervals

        if (!merged.empty() &&
            std::get<0>(merged.back()) == pid &&
            std::get<2>(merged.back()) == start) {
            std::get<2>(merged.back()) = end; // Extend interval
        } else {
            merged.push_back(entry);
        }
    }
    return merged;
}

void GanttChart::renderTimeline(const std::vector<GanttEntry>& rawGantt,
                                std::ostream& out,
                                bool useColors) {
    auto merged = mergeAdjacent(rawGantt);
    if (merged.empty()) {
        out << "  [Empty Gantt Chart]\n";
        return;
    }

    out << "  Timeline: ";
    for (size_t i = 0; i < merged.size(); ++i) {
        const auto& [pid, start, end] = merged[i];
        out << colorFor(pid, useColors)
            << labelFor(pid) << " [" << start << "-" << end << "]"
            << resetCol(useColors);

        if (i + 1 < merged.size()) {
            out << " -> ";
        }
    }
    out << "\n";
}

void GanttChart::renderBarChart(const std::vector<GanttEntry>& rawGantt,
                                std::ostream& out,
                                int maxWidth,
                                bool useColors) {
    auto merged = mergeAdjacent(rawGantt);
    if (merged.empty()) {
        out << "  [Empty Gantt Chart]\n";
        return;
    }

    if (maxWidth < 40) maxWidth = 80;

    // Split entries into row chunks so long timelines wrap cleanly without breaking terminal
    size_t idx = 0;
    while (idx < merged.size()) {
        std::vector<GanttEntry> rowEntries;
        int currentChars = 2; // Leading indentation

        while (idx < merged.size()) {
            const auto& [pid, start, end] = merged[idx];
            int duration = end - start;
            std::string label = labelFor(pid);
            // Block width proportional to duration, minimum width to fit label
            int blockWidth = std::max(static_cast<int>(label.length()) + 2, std::min(duration * 2 + 2, 14));

            if (!rowEntries.empty() && (currentChars + blockWidth + 1) > maxWidth) {
                break; // Wrap to next row
            }
            rowEntries.push_back(merged[idx]);
            currentChars += blockWidth + 1;
            ++idx;
        }

        if (rowEntries.empty()) break;

        // 1. Top border
        out << "  ";
        for (const auto& entry : rowEntries) {
            int pid = std::get<0>(entry);
            int start = std::get<1>(entry);
            int end = std::get<2>(entry);
            int duration = end - start;
            std::string label = labelFor(pid);
            int blockWidth = std::max(static_cast<int>(label.length()) + 2, std::min(duration * 2 + 2, 14));
            out << "+" << std::string(blockWidth, '-');
        }
        out << "+\n";

        // 2. Process Labels
        out << "  ";
        for (const auto& entry : rowEntries) {
            int pid = std::get<0>(entry);
            int start = std::get<1>(entry);
            int end = std::get<2>(entry);
            int duration = end - start;
            std::string label = labelFor(pid);
            int blockWidth = std::max(static_cast<int>(label.length()) + 2, std::min(duration * 2 + 2, 14));

            int padLeft = (blockWidth - static_cast<int>(label.length())) / 2;
            int padRight = blockWidth - static_cast<int>(label.length()) - padLeft;

            out << "|"
                << colorFor(pid, useColors)
                << std::string(padLeft, ' ') << label << std::string(padRight, ' ')
                << resetCol(useColors);
        }
        out << "|\n";

        // 3. Bottom border
        out << "  ";
        for (const auto& entry : rowEntries) {
            int pid = std::get<0>(entry);
            int start = std::get<1>(entry);
            int end = std::get<2>(entry);
            int duration = end - start;
            std::string label = labelFor(pid);
            int blockWidth = std::max(static_cast<int>(label.length()) + 2, std::min(duration * 2 + 2, 14));
            out << "+" << std::string(blockWidth, '-');
        }
        out << "+\n";

        // 4. Timestamp Markers
        out << "  ";
        for (size_t k = 0; k < rowEntries.size(); ++k) {
            int start = std::get<1>(rowEntries[k]);
            std::string startStr = std::to_string(start);

            int pid = std::get<0>(rowEntries[k]);
            int end = std::get<2>(rowEntries[k]);
            int duration = end - start;
            std::string label = labelFor(pid);
            int blockWidth = std::max(static_cast<int>(label.length()) + 2, std::min(duration * 2 + 2, 14));

            out << startStr;
            int remainingSpace = (blockWidth + 1) - static_cast<int>(startStr.length());
            if (remainingSpace > 0) {
                out << std::string(remainingSpace, ' ');
            }
        }
        // Print the very last end timestamp of this row
        out << std::get<2>(rowEntries.back()) << "\n\n";
    }
}

void GanttChart::renderIntervalDetails(const std::vector<GanttEntry>& rawGantt,
                                       std::ostream& out,
                                       bool useColors) {
    auto merged = mergeAdjacent(rawGantt);
    if (merged.empty()) return;

    std::map<int, std::vector<std::pair<int, int>>> intervalsByPid;
    int idleTotal = 0;
    int csTotal = 0;

    for (const auto& [pid, s, e] : merged) {
        if (pid == -1) {
            idleTotal += (e - s);
        } else if (pid == -2) {
            csTotal += (e - s);
        } else {
            intervalsByPid[pid].emplace_back(s, e);
        }
    }

    out << "  Execution Slices per Process:\n";
    for (const auto& [pid, intervals] : intervalsByPid) {
        int totalTime = 0;
        out << "    " << colorFor(pid, useColors) << "P" << pid << resetCol(useColors) << ": ";
        for (size_t i = 0; i < intervals.size(); ++i) {
            out << "[" << intervals[i].first << " -> " << intervals[i].second << "]";
            totalTime += (intervals[i].second - intervals[i].first);
            if (i + 1 < intervals.size()) out << ", ";
        }
        out << " (Active Time: " << totalTime
            << ", Slices: " << intervals.size();
        if (intervals.size() > 1) {
            out << ", Preemptions: " << (intervals.size() - 1);
        }
        out << ")\n";
    }

    if (csTotal > 0) {
        out << "    " << colorFor(-2, useColors) << "Context Switching" << resetCol(useColors)
            << ": Total Overhead = " << csTotal << " units\n";
    }
    if (idleTotal > 0) {
        out << "    " << colorFor(-1, useColors) << "CPU Idle" << resetCol(useColors)
            << ": Total Idle Time = " << idleTotal << " units\n";
    }
    out << "\n";
}

void GanttChart::render(const std::vector<GanttEntry>& rawGantt,
                        std::ostream& out,
                        int maxWidth,
                        bool useColors) {
    out << "\n=== Gantt Chart Visualization ===\n\n";
    renderBarChart(rawGantt, out, maxWidth, useColors);
    renderTimeline(rawGantt, out, useColors);
    out << "\n";
    renderIntervalDetails(rawGantt, out, useColors);
}

} // namespace SchedOS
