#pragma once

#include "schedos/Scheduler.h"
#include <vector>
#include <string>
#include <iosfwd>

namespace SchedOS {

class GanttChart {
public:
    /// Merges contiguous intervals of the same process / state.
    static std::vector<GanttEntry> mergeAdjacent(const std::vector<GanttEntry>& raw);

    /// Prints a compact arrow timeline: P1 [0-3] -> P2 [3-5] -> ...
    static void renderTimeline(const std::vector<GanttEntry>& rawGantt,
                               std::ostream& out,
                               bool useColors = true);

    /// Prints an ASCII visual bar chart with aligned timestamps.
    /// Wraps lines if total character width exceeds maxWidth.
    static void renderBarChart(const std::vector<GanttEntry>& rawGantt,
                               std::ostream& out,
                               int maxWidth = 80,
                               bool useColors = true);

    /// Prints detailed interval analysis and preemption counts per process.
    static void renderIntervalDetails(const std::vector<GanttEntry>& rawGantt,
                                      std::ostream& out,
                                      bool useColors = true);

    /// Full combined render (Bar chart + Timeline + Details).
    static void render(const std::vector<GanttEntry>& rawGantt,
                       std::ostream& out,
                       int maxWidth = 80,
                       bool useColors = true);
};

} // namespace SchedOS
