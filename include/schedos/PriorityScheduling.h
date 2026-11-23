#pragma once

#include "schedos/Scheduler.h"

namespace SchedOS {

class PriorityScheduling : public Scheduler {
public:
    void schedule(std::vector<Process>& processes) override;
    std::string name() const override {
        return (agingInterval_ > 0) ? "Priority Scheduling (Non-Preemptive with Aging)"
                                    : "Priority Scheduling (Non-Preemptive)";
    }

    /// Sets aging interval. If > 0, priority value improves by 1 for every
    /// agingInterval time units spent waiting in the ready queue.
    void setAgingInterval(int interval) { agingInterval_ = (interval >= 0) ? interval : 0; }
    int agingInterval() const { return agingInterval_; }

private:
    int agingInterval_ = 0;
};

} // namespace SchedOS
