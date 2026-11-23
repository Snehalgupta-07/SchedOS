#include "schedos/ProcessManager.h"
#include <algorithm>
#include <stdexcept>

namespace SchedOS {

void ProcessManager::addProcess(const Process& p) {
    p.validate();
    if (hasPid(p.pid)) {
        throw std::invalid_argument("Process with PID " + std::to_string(p.pid) + " already exists.");
    }
    processes_.push_back(p);
}

bool ProcessManager::editProcess(int pid, int newArrival, int newBurst, int newPriority) {
    for (auto& p : processes_) {
        if (p.pid == pid) {
            Process temp(pid, newArrival, newBurst, newPriority);
            temp.validate();
            p = temp;
            return true;
        }
    }
    return false;
}

bool ProcessManager::deleteProcess(int pid) {
    auto it = std::remove_if(processes_.begin(), processes_.end(),
                             [pid](const Process& p) { return p.pid == pid; });
    if (it != processes_.end()) {
        processes_.erase(it, processes_.end());
        return true;
    }
    return false;
}

std::optional<Process> ProcessManager::findProcess(int pid) const {
    for (const auto& p : processes_) {
        if (p.pid == pid) return p;
    }
    return std::nullopt;
}

void ProcessManager::clear() {
    processes_.clear();
}

void ProcessManager::setProcesses(const std::vector<Process>& procs) {
    // Check for duplicates within incoming list
    for (size_t i = 0; i < procs.size(); ++i) {
        procs[i].validate();
        for (size_t j = i + 1; j < procs.size(); ++j) {
            if (procs[i].pid == procs[j].pid) {
                throw std::invalid_argument("Duplicate PID " + std::to_string(procs[i].pid) + " in process list.");
            }
        }
    }
    processes_ = procs;
}

void ProcessManager::sortByArrival() {
    std::stable_sort(processes_.begin(), processes_.end(),
        [](const Process& a, const Process& b) {
            if (a.arrivalTime != b.arrivalTime)
                return a.arrivalTime < b.arrivalTime;
            return a.pid < b.pid;
        });
}

bool ProcessManager::hasPid(int pid) const {
    for (const auto& p : processes_) {
        if (p.pid == pid) return true;
    }
    return false;
}

} // namespace SchedOS
