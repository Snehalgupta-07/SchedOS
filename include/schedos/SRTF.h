#pragma once

#include "schedos/Scheduler.h"

namespace SchedOS {

class SRTF : public Scheduler {
public:
    void schedule(std::vector<Process>& processes) override;
    std::string name() const override { return "Shortest Remaining Time First (SRTF)"; }
};

} // namespace SchedOS
