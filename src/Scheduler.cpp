#include "schedos/Scheduler.h"
#include <algorithm>

namespace SchedOS {

void Scheduler::sortByArrival(std::vector<Process>& procs) {
    std::stable_sort(procs.begin(), procs.end(),
        [](const Process& a, const Process& b) {
            if (a.arrivalTime != b.arrivalTime)
                return a.arrivalTime < b.arrivalTime;
            return a.pid < b.pid;
        });
}

} // namespace SchedOS
