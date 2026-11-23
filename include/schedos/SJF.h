#pragma once

#include "schedos/Scheduler.h"

namespace SchedOS {

class SJF : public Scheduler {
public:
    void schedule(std::vector<Process>& processes) override;
    std::string name() const override { return "Shortest Job First (SJF)"; }
};

} // namespace SchedOS
