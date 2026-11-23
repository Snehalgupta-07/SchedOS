# SchedOS — CPU Scheduling and Process Management Simulator

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-3.14%2B-brightgreen.svg)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Tests](https://img.shields.io/badge/Tests-69%20passed-success.svg)](#automated-testing)

**SchedOS** is a high-performance, modular operating system process scheduling engine engineered in modern C++17. Designed with strict clean-code principles, object-oriented abstractions, and industry-standard algorithms, SchedOS models CPU scheduling dynamics, context switching, preemption, idle periods, and system performance metrics with mathematical precision.

Developed as a core systems-programming portfolio project targeting high-reliability software engineering roles (such as Siemens Software Engineer / Graduate Engineer Trainee).

---

## Architecture Overview

SchedOS adheres to modular separation of concerns:

```
SchedOS/
├── CMakeLists.txt              # Build configuration with C++17 & warning flags
├── .gitignore                  # Git exclusions for build artifacts & temp files
├── include/
│   └── schedos/
│       ├── Process.h           # Process model, validation, and table formatting
│       ├── Scheduler.h         # Abstract Scheduler base class & Gantt definitions
│       ├── FCFS.h              # First-Come First-Served algorithm
│       ├── SJF.h               # Shortest Job First (non-preemptive)
│       ├── SRTF.h              # Shortest Remaining Time First (preemptive)
│       ├── RoundRobin.h        # Round Robin with configurable quantum
│       ├── PriorityScheduling.h# Priority Scheduling (non-preemptive)
│       └── Metrics.h           # Aggregate performance metrics calculations
├── src/
│   ├── Process.cpp             # Process implementation & I/O stream parser
│   ├── Scheduler.cpp           # Shared sorting & scheduler utilities
│   ├── FCFS.cpp                # FCFS scheduling implementation
│   ├── SJF.cpp                 # SJF scheduling implementation
│   ├── SRTF.cpp                # SRTF preemptive scheduling implementation
│   ├── RoundRobin.cpp          # Round Robin implementation with ready-queue
│   ├── PriorityScheduling.cpp  # Priority scheduling implementation
│   ├── Metrics.cpp             # Turnaround, Waiting, Utilization, Throughput
│   └── main.cpp                # CLI interface for simulation execution
├── samples/                    # Verification sample workloads
│   ├── sample_input.txt        # Multi-process standard workload
│   ├── simultaneous_arrival.txt# Simultaneous arrival benchmark
│   ├── single_process.txt      # Boundary/edge-case benchmark
│   └── with_idle_gaps.txt      # Idle-gap handling verification
└── tests/
    └── test_main.cpp           # Automated unit & integration test suite (69 checks)
```

---

## Implemented Scheduling Algorithms

SchedOS implements five fundamental operating system CPU scheduling algorithms:

| Algorithm | Type | Preemptive | Criteria / Dispatch Order |
|:---|:---:|:---:|:---|
| **First-Come, First-Served (FCFS)** | Batch | No | Arrival Time ($t_{\text{arr}}$) |
| **Shortest Job First (SJF)** | Batch | No | Burst Time ($t_{\text{burst}}$) among arrived jobs |
| **Shortest Remaining Time First (SRTF)** | Interactive / Real-time | **Yes** | Remaining Burst Time ($t_{\text{rem}}$) with dynamic preemption |
| **Round Robin (RR)** | Time-sharing | **Yes** | Configurable Time Quantum ($q$) with FIFO ready queue |
| **Priority Scheduling** | Priority-based | No | Priority value (lower numerical value = higher priority) |

---

## Performance Metrics Computed

For every individual process and overall system schedule:

- **Completion Time ($CT$)**: Timestamp when process completes execution.
- **Turnaround Time ($TAT$)**: Total elapsed time from arrival to completion ($TAT = CT - AT$).
- **Waiting Time ($WT$)**: Total time spent in the ready queue waiting for the CPU ($WT = TAT - BT$).
- **Response Time ($RT$)**: Duration from process arrival until first CPU allocation ($RT = \text{FirstExecution} - AT$).
- **Average Waiting Time**: $\frac{1}{N} \sum WT_i$
- **Average Turnaround Time**: $\frac{1}{N} \sum TAT_i$
- **Average Response Time**: $\frac{1}{N} \sum RT_i$
- **CPU Utilization**: $\frac{\text{Total Busy Time}}{\text{Total Elapsed Time}} \times 100\%$
- **Throughput**: $\frac{N}{\text{Total Simulation Time}}$ processes/unit time
- **Gantt Chart Intervals**: Full execution intervals logged for Phase 2 visualizer integration.

---

## Edge Case & Fault Tolerance Handling

- **Simultaneous Arrivals**: Tie-breaking deterministic ordering by Process ID (PID).
- **CPU Idle Periods**: Seamless handling of arrival gaps; idle periods logged with `PID = -1`.
- **Preemption Granularity**: Unit-step preemption checks in SRTF with automated interval merging.
- **Round Robin Quantum Arrivals**: Correct queuing order when a process finishes its slice at the exact moment a new process arrives.
- **Data Validation**: Strict input boundary checks throwing `std::invalid_argument` for negative PIDs, negative arrival times, or non-positive bursts.

---

## Build & Run Instructions

### Prerequisites
- C++17 compliant compiler (`g++` 8+, `clang++` 7+, or MSVC 2019+)
- CMake 3.14+

### Building with CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

On Windows with MinGW Makefiles:
```powershell
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

### Running the Simulator

```bash
# Run all algorithms on sample workload
./schedos -f ../samples/sample_input.txt -a all -q 2

# Run specific algorithm (e.g. SRTF)
./schedos -f ../samples/sample_input.txt -a srtf

# Run Round Robin with custom quantum
./schedos -f ../samples/sample_input.txt -a rr -q 3
```

### Running Automated Tests

```bash
# Direct test executable execution
./schedos_tests

# Or via CTest
ctest --output-on-failure
```

---

## Automated Test Coverage

The test suite covers:
- **FCFS Tests**: Standard execution, simultaneous arrival ordering, CPU idle intervals, single-process edge case.
- **SJF Tests**: Standard execution, burst time tie-breaking.
- **SRTF Tests**: Preemption on shorter job arrival, run-to-completion without preemption.
- **Round Robin Tests**: Quantum progression, quantum = 1, quantum > burst time, invalid quantum exception.
- **Priority Scheduling Tests**: Standard prioritization, equal priority tie-breaking.
- **Metrics Tests**: Statistical verification of average WT, TAT, CPU utilization, throughput.
- **Data Model & Edge Cases**: Input validation exceptions, empty schedule handling, comments/whitespace handling in stream parser.

```
========================================
   SchedOS — Scheduling Test Suite     
========================================

--- FCFS Tests ---
--- SJF Tests ---
--- SRTF Tests ---
--- Round Robin Tests ---
--- Priority Scheduling Tests ---
--- Metrics Tests ---
--- Edge Case Tests ---

========================================
  Results: 69 passed, 0 failed, 69 total
========================================
```

---

## Project Status

- [x] **Phase 1: Core C++ Scheduling Engine** (Completed)
  - [x] Process model & validation
  - [x] FCFS & SJF algorithms
  - [x] SRTF preemptive scheduling
  - [x] Round Robin & Priority scheduling
  - [x] Comprehensive performance metrics & Gantt logging
  - [x] Modular CMake configuration
  - [x] Automated test harness (69/69 passing)
  - [x] Sample benchmarks & CLI driver
- [ ] **Phase 2: Visualizations & Advanced Features** (Upcoming)
