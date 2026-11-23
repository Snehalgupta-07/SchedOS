#pragma once

#include "schedos/Scheduler.h"

namespace SchedOS {

class PriorityScheduling : public Scheduler {
public:
    void schedule(std::vector<Process>& processes) override;
    std::string name() const override { return "Priority Scheduling (Non-Preemptive)"; }
};

} // namespace SchedOS
