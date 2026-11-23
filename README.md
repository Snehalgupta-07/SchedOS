# SchedOS — CPU Scheduling and Process Management Simulator

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-3.14%2B-brightgreen.svg)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Tests](https://img.shields.io/badge/Tests-148%20passed-success.svg)](#automated-testing)

**SchedOS** is a high-performance, modular operating system process scheduling engine and interactive simulation platform engineered in modern C++17. Designed with strict clean-code principles, object-oriented abstractions, and industry-standard operating system scheduling algorithms, SchedOS models CPU scheduling dynamics, preemptions, CPU idle periods, configurable context-switching overhead, and system performance metrics with mathematical precision.

Developed as a core systems-programming portfolio project targeting high-reliability software engineering roles (such as Siemens Software Engineer / Graduate Engineer Trainee).

---

## Key Features

- **5 Core CPU Scheduling Algorithms**: FCFS, SJF (Non-preemptive), SRTF (Preemptive), Round Robin (Preemptive with configurable quantum), Priority Scheduling (Non-preemptive).
- **Realistic Context-Switching Simulation**: Configurable context-switch cost ($C$). Correctly models CPU time consumed during task switching, handles process arrivals during context switches, and distinguishes productive CPU utilization from total CPU occupancy.
- **Terminal Gantt Chart Visualization**: Renders ASCII bar charts with aligned time markers and conceptual arrow execution timelines (`P1 [0–3] -> CS [3–4] -> P2 [4–8]`). Automatically wraps long timelines without breaking terminal formatting.
- **Algorithm Performance Comparison Benchmark**: Executes all 5 algorithms on identical process workloads, highlights best-performing algorithms per metric, and articulates operating systems trade-off analysis.
- **Interactive Menu-Driven CLI**: 11-option interactive terminal interface with ANSI color highlights, robust input validation, and support for repeated in-memory simulations without restarting.
- **CSV Import & Export Engine**: Validates missing fields, non-numeric values, duplicate PIDs, negative arrival times, and non-positive burst times. Exports process metrics and comparison benchmarks to CSV.
- **Zero External Dependencies**: Pure C++17 standard library implementation (`<vector>`, `<queue>`, `<memory>`, `<iostream>`, `<fstream>`).

---

## Architecture Overview

SchedOS adheres to modular separation of concerns:

```
SchedOS/
├── CMakeLists.txt                  # Build configuration with C++17, warning flags, static linking
├── .gitignore                      # Git exclusions for build artifacts & temporary files
├── README.md                       # Complete technical documentation
├── include/
│   └── schedos/
│       ├── Process.h               # Process data model, validation, and table formatting
│       ├── ProcessManager.h        # In-memory process store (Add/Edit/Delete/Find/Sort)
│       ├── Scheduler.h             # Abstract Scheduler base class & Gantt definitions
│       ├── FCFS.h                  # First-Come First-Served algorithm
│       ├── SJF.h                   # Shortest Job First (non-preemptive)
│       ├── SRTF.h                  # Shortest Remaining Time First (preemptive)
│       ├── RoundRobin.h            # Round Robin with configurable quantum
│       ├── PriorityScheduling.h    # Priority Scheduling (non-preemptive)
│       ├── Metrics.h               # Aggregate performance metrics calculations
│       ├── GanttChart.h            # ASCII Gantt bar chart & arrow timeline visualizer
│       ├── ComparisonEngine.h      # Multi-algorithm benchmark & trade-off analysis
│       ├── CSVManager.h            # Robust CSV parsing, validation, and export
│       └── CLI.h                   # Interactive menu-driven console application
├── src/
│   ├── Process.cpp                 # Process validation & stream parser
│   ├── ProcessManager.cpp          # Process collection management
│   ├── Scheduler.cpp               # Shared sorting & scheduler utilities
│   ├── FCFS.cpp                    # FCFS scheduling with context-switch handling
│   ├── SJF.cpp                     # SJF scheduling with context-switch handling
│   ├── SRTF.cpp                    # SRTF preemptive scheduling with interval merging
│   ├── RoundRobin.cpp              # Round Robin ready queue & slice management
│   ├── PriorityScheduling.cpp      # Priority scheduling implementation
│   ├── Metrics.cpp                 # Statistical analysis & CPU metrics computation
│   ├── GanttChart.cpp              # Gantt chart rendering & preemption tracking
│   ├── ComparisonEngine.cpp        # Benchmark execution & trade-off reporting
│   ├── CSVManager.cpp              # CSV reader/writer with strict schema checks
│   ├── CLI.cpp                     # Interactive terminal menus and input handling
│   └── main.cpp                    # CLI entrypoint (interactive or batch execution)
├── samples/                        # Verification workloads
│   ├── processes.csv               # Standard CSV benchmark dataset
│   ├── workload_heavy.csv          # High-volume process batch
│   ├── sample_input.txt            # Text-formatted multi-process benchmark
│   ├── simultaneous_arrival.txt    # Simultaneous arrival benchmark
│   ├── single_process.txt          # Boundary/edge-case benchmark
│   └── with_idle_gaps.txt          # CPU idle-gap handling verification
└── tests/
    └── test_main.cpp               # Comprehensive automated test suite (148 tests)
```

---

## Implemented Scheduling Algorithms

| Algorithm | Type | Preemptive | Criteria / Dispatch Order |
|:---|:---:|:---:|:---|
| **First-Come, First-Served (FCFS)** | Batch | No | Arrival Time ($t_{\text{arr}}$) |
| **Shortest Job First (SJF)** | Batch | No | Burst Time ($t_{\text{burst}}$) among arrived jobs |
| **Shortest Remaining Time First (SRTF)** | Interactive / Real-time | **Yes** | Remaining Burst Time ($t_{\text{rem}}$) with dynamic preemption |
| **Round Robin (RR)** | Time-sharing | **Yes** | Configurable Time Quantum ($q$) with FIFO ready queue |
| **Priority Scheduling** | Priority-based | No | Priority value (lower numerical value = higher priority) |

---

## Context-Switching Simulation ($C$)

In operating systems, switching the CPU between two active tasks requires saving the current registers, program counter, and stack pointer, and restoring the new process context.

### Charging Rules
1. **Inter-Process Switching ($P_A \to P_B, P_A \neq P_B, P_A \ge 0, P_B \ge 0$):** Incurs context-switch overhead $C$.
2. **Idle-to-Process ($P_{\text{idle}} \to P_{\text{next}}$):** When waking from an idle state, no prior process context requires saving, so no inter-process context switch is charged.
3. **Same Process Re-selection ($P_A \to P_A$):** If Round Robin re-selects the sole process in the ready queue, or SRTF continues the same process, no context switch overhead is charged.
4. **Time Advancement:** Time elapses during context switches (`currentTime += C`). Any processes arriving during this window are queued and become eligible for dispatch at the completion of the context switch.

---

## Performance Metrics Computed

- **Completion Time ($CT$)**: Timestamp when process completes execution.
- **Turnaround Time ($TAT$)**: Total elapsed time from arrival to completion ($TAT = CT - AT$).
- **Waiting Time ($WT$)**: Total time spent waiting in the ready queue ($WT = TAT - BT$).
- **Response Time ($RT$)**: Duration from process arrival until first CPU allocation ($RT = t_{\text{first}} - AT$).
- **Average Waiting Time**: $\frac{1}{N} \sum WT_i$
- **Average Turnaround Time**: $\frac{1}{N} \sum TAT_i$
- **Average Response Time**: $\frac{1}{N} \sum RT_i$
- **Productive CPU Utilization**: $\frac{\text{Total Busy Time}}{\text{Total Simulation Duration}} \times 100\%$
- **Total CPU Occupancy**: $\frac{\text{Total Busy Time} + \text{Total Context Switch Overhead}}{\text{Total Simulation Duration}} \times 100\%$
- **Throughput**: $\frac{N}{\text{Total Simulation Duration}}$ processes/unit time

---

## Interactive Command-Line Interface

Launch without arguments to start the interactive console:

```
========================================================
  SchedOS — CPU Scheduling & Process Management Simulator
========================================================
  Active Algorithm: fcfs | RR Quantum: 2 | CS Overhead: 0 | Processes: 5
--------------------------------------------------------
  1. Add a new process
  2. View all processes
  3. Edit an existing process
  4. Delete a process
  5. Select a scheduling algorithm
  6. Execute scheduling simulation
  7. Compare all scheduling algorithms
  8. Load processes from CSV
  9. Export results to CSV
  10. Reset simulation
  11. Exit
--------------------------------------------------------
  Enter your choice (1-11):
```

---

## Gantt Chart Visualization Example

Output from `schedos -f samples/sample_input.txt -a fcfs`:

```
=== Gantt Chart Visualization ===

  +--------------+------------+--------------+--------+--------------+
  |      P1      |     P2     |      P3      |   P4   |      P5      |
  +--------------+------------+--------------+--------+--------------+
  0              10           15             23       26             33

  Timeline: P1 [0-10] -> P2 [10-15] -> P3 [15-23] -> P4 [23-26] -> P5 [26-33]

  Execution Slices per Process:
    P1: [0 -> 10] (Active Time: 10, Slices: 1)
    P2: [10 -> 15] (Active Time: 5, Slices: 1)
    P3: [15 -> 23] (Active Time: 8, Slices: 1)
    P4: [23 -> 26] (Active Time: 3, Slices: 1)
    P5: [26 -> 33] (Active Time: 7, Slices: 1)
```

---

## Algorithm Performance Comparison Example

```
====================================================================================================
                             ALGORITHM PERFORMANCE COMPARISON BENCHMARK                             
====================================================================================================
Algorithm                       Avg WT    Avg TAT   Avg RT    Prod. CPU(%)  Throughput    CS Count  Total Time
----------------------------------------------------------------------------------------------------
First Come First Serve (FCFS)   12.60     19.20     12.60     100.0%*       0.1515*       4*        33*       
Shortest Job First (SJF)        11.00     17.60     11.00     100.0%*       0.1515*       4*        33*       
Shortest Remaining Time First   8.80*     15.40*    4.20      100.0%*       0.1515*       5         33*       
Round Robin (RR)                18.00     24.60     3.00*     100.0%*       0.1515*       17        33*       
Priority Scheduling (Non-Pre)   13.60     20.20     13.60     100.0%*       0.1515*       4*        33*       
----------------------------------------------------------------------------------------------------
  * Indicates best performance for that metric.
```

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

On Windows with MinGW:
```powershell
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

### Running

```bash
# Launch interactive menu CLI
./schedos

# Batch execution on sample CSV with all algorithms
./schedos -f ../samples/processes.csv -a all -q 2 -c 1

# Specific algorithm with Round Robin quantum 3 and context-switch cost 1
./schedos -f ../samples/processes.csv -a rr -q 3 -c 1
```

### Running Automated Tests

```bash
# Direct test executable
./schedos_tests

# Or via CTest
ctest --output-on-failure
```

---

## Automated Test Coverage

The test harness exercises 148 automated assertions across 8 distinct suites:

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
--- ProcessManager Tests ---
--- GanttChart Tests ---
--- Context Switching Tests ---
--- ComparisonEngine Tests ---
--- CSVManager Tests ---
--- Advanced Verification Tests ---

========================================
  Results: 148 passed, 0 failed, 148 total
========================================
```

---

## Project Status

- [x] **Phase 1: Core C++ Scheduling Engine** (Completed)
  - [x] Process model & validation
  - [x] FCFS & SJF algorithms
  - [x] SRTF preemptive scheduling
  - [x] Round Robin & Priority scheduling
  - [x] Scheduling metrics calculations
  - [x] CMake build configuration
- [x] **Phase 2: Advanced Scheduling, Visualization & Interactive CLI** (Completed)
  - [x] Interactive menu-driven console (11 options)
  - [x] ProcessManager (CRUD operations, duplicate PID detection)
  - [x] Configurable context-switch simulation & overhead tracking
  - [x] Gantt Chart terminal visualizer (ASCII bar chart + arrow timeline)
  - [x] Multi-algorithm comparison benchmark with best-metric highlighting
  - [x] CSV Import / Export engine with strict data validation
  - [x] Mathematical separation of Productive CPU Util vs Total Occupancy
  - [x] 148 automated tests covering all edge cases
