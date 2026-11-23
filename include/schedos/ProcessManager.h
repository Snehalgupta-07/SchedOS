#pragma once

#include "schedos/Process.h"
#include <vector>
#include <optional>
#include <string>

namespace SchedOS {

class ProcessManager {
public:
    ProcessManager() = default;

    /// Adds a process. Throws std::invalid_argument if PID already exists or validation fails.
    void addProcess(const Process& p);

    /// Edits an existing process by PID. Returns true on success, false if not found.
    bool editProcess(int pid, int newArrival, int newBurst, int newPriority);

    /// Deletes a process by PID. Returns true on success, false if not found.
    bool deleteProcess(int pid);

    /// Finds a process by PID.
    std::optional<Process> findProcess(int pid) const;

    /// Clears all processes.
    void clear();

    /// Returns all current processes.
    const std::vector<Process>& getProcesses() const { return processes_; }

    /// Sets the entire process list (validates for duplicate PIDs).
    void setProcesses(const std::vector<Process>& procs);

    /// Returns the number of processes.
    size_t size() const { return processes_.size(); }

    /// Returns whether the process list is empty.
    bool empty() const { return processes_.empty(); }

    /// Sorts processes by arrival time (and PID).
    void sortByArrival();

    /// Checks if a PID exists.
    bool hasPid(int pid) const;

private:
    std::vector<Process> processes_;
};

} // namespace SchedOS
