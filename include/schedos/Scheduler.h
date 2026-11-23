#pragma once

#include "schedos/Process.h"
#include <vector>
#include <string>
#include <tuple>

namespace SchedOS {

/// Gantt chart entry: (process_id, start_time, end_time). pid = -1 means idle.
using GanttEntry = std::tuple<int, int, int>;

/// Abstract base class for all scheduling algorithms.
class Scheduler {
public:
    virtual ~Scheduler() = default;

    /// Schedules the given processes and populates their timing fields.
    virtual void schedule(std::vector<Process>& processes) = 0;

    /// Returns the name of the algorithm.
    virtual std::string name() const = 0;

    /// Returns the Gantt chart built during the last schedule() call.
    const std::vector<GanttEntry>& ganttChart() const { return gantt_; }

protected:
    std::vector<GanttEntry> gantt_;

    /// Sorts processes by arrival time (stable, breaking ties by PID).
    static void sortByArrival(std::vector<Process>& procs);
};

} // namespace SchedOS
