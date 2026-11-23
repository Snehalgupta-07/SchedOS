#pragma once

#include "schedos/Scheduler.h"

namespace SchedOS {

class RoundRobin : public Scheduler {
public:
    explicit RoundRobin(int timeQuantum = 2);

    void schedule(std::vector<Process>& processes) override;
    std::string name() const override { return "Round Robin (RR)"; }

    int timeQuantum() const { return quantum_; }
    void setTimeQuantum(int q);

private:
    int quantum_;
};

} // namespace SchedOS
