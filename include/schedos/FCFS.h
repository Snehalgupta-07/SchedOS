#pragma once

#include "schedos/Scheduler.h"

namespace SchedOS {

class FCFS : public Scheduler {
public:
    void schedule(std::vector<Process>& processes) override;
    std::string name() const override { return "First Come First Serve (FCFS)"; }
};

} // namespace SchedOS
