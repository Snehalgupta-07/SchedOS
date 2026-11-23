#pragma once

#include "schedos/Process.h"
#include <vector>
#include <string>
#include <tuple>

namespace SchedOS {

/// Gantt chart entry: (process_id, start_time, end_time).
/// pid >= 0 : Process execution
/// pid == -1: CPU Idle
/// pid == -2: Context Switch Overhead
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

    /// Context switch configuration
    void setContextSwitchCost(int cost) { contextSwitchCost_ = (cost >= 0) ? cost : 0; }
    int contextSwitchCost() const { return contextSwitchCost_; }

    /// Metrics related to context switching
    int contextSwitchCount() const { return contextSwitchCount_; }
    int totalContextSwitchOverhead() const { return totalContextSwitchOverhead_; }

protected:
    std::vector<GanttEntry> gantt_;
    int contextSwitchCost_ = 0;
    int contextSwitchCount_ = 0;
    int totalContextSwitchOverhead_ = 0;

    /// Sorts processes by arrival time (stable, breaking ties by PID).
    static void sortByArrival(std::vector<Process>& procs);

    /// Resets scheduler state prior to scheduling.
    void resetSchedulerState() {
        gantt_.clear();
        contextSwitchCount_ = 0;
        totalContextSwitchOverhead_ = 0;
    }
};

} // namespace SchedOS
