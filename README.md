# SchedOS — CPU Scheduling and Process Management Simulator

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-3.14%2B-brightgreen.svg)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Tests](https://img.shields.io/badge/Tests-203%20passed-success.svg)](#automated-testing)

**SchedOS** is a high-performance, modular operating system process scheduling engine, concurrency benchmarking framework, and interactive simulation platform engineered in modern C++17. Designed with strict clean-code principles, object-oriented abstractions, and industry-standard operating system scheduling algorithms, SchedOS models CPU scheduling dynamics, preemptions, CPU idle periods, configurable context-switching overhead, and system performance metrics with mathematical precision.

Beyond mathematical scheduling simulation, SchedOS incorporates an enterprise-grade **native C++ multithreading and concurrency subsystem** featuring a thread-safe task queue, a reusable RAII-compliant worker thread pool, and a high-precision performance benchmarking engine that measures parallel speedup and throughput under real CPU-bound workloads.

Developed as a core systems-programming portfolio project targeting high-reliability software engineering roles (such as Siemens Software Engineer / Graduate Engineer Trainee).

---

## Key Features

- **5 Core CPU Scheduling Algorithms**: FCFS, SJF (Non-preemptive), SRTF (Preemptive), Round Robin (Preemptive with configurable quantum), Priority Scheduling (Non-preemptive).
- **Dynamic Priority Aging**: Prevents process starvation in priority scheduling by dynamically boosting waiting process priorities over configurable time intervals ($P_{\text{eff}} = \max(0, P_{\text{base}} - \lfloor \frac{\text{Wait}}{\text{Interval}} \rfloor)$).
- **Scheduling Fairness & Dispersion Metrics**: Computes Waiting-Time Variance ($\sigma^2$), Standard Deviation ($\sigma$), and Min/Max waiting times to evaluate scheduling fairness.
- **Realistic Context-Switching Simulation**: Configurable context-switch cost ($C$). Correctly models CPU time consumed during task switching, handles process arrivals during context switches, and distinguishes productive CPU utilization from total CPU occupancy.
- **Native C++ Multithreading & Thread Pool**: Industrial-grade worker pool (`std::thread`, `std::mutex`, `std::condition_variable`, `std::atomic`, `std::future`) with thread-safe task submission, task tracking, exception propagation, and graceful RAII shutdown.
- **Performance Benchmarking Suite**: Measures sequential vs. multithreaded speedup and throughput on repeatable CPU-bound workloads using `std::chrono::steady_clock`, volatile accumulators, warm-up runs, and median filtering.
- **Terminal Gantt Chart Visualization**: Renders ASCII bar charts with aligned time markers and conceptual arrow execution timelines (`P1 [0–3] -> CS [3–4] -> P2 [4–8]`). Automatically wraps long timelines without breaking terminal formatting.
- **Algorithm Performance Comparison Benchmark**: Executes all 5 algorithms on identical process workloads, highlights best-performing algorithms per metric, and articulates operating systems trade-off analysis.
- **Interactive Menu-Driven CLI**: 12-option interactive terminal interface (including a full Concurrency & Benchmarking submenu) with ANSI color highlights, robust input validation, and repeated in-memory simulations without restarting.
- **CSV Import & Export Engine**: Validates missing fields, non-numeric values, duplicate PIDs, negative arrival times, and non-positive burst times. Exports process metrics and comparison benchmarks to CSV.
- **High-Capacity Scalability**: Tested and verified under stress workloads up to 10,000 processes and high-concurrency multi-producer thread pool operations.
- **Zero External Dependencies**: Pure C++17 standard library implementation (`<vector>`, `<queue>`, `<memory>`, `<thread>`, `<mutex>`, `<condition_variable>`, `<atomic>`, `<future>`, `<chrono>`).

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
│       ├── PriorityScheduling.h    # Priority Scheduling with aging support
│       ├── Metrics.h               # Aggregate performance & fairness metrics
│       ├── GanttChart.h            # ASCII Gantt bar chart & arrow timeline visualizer
│       ├── ComparisonEngine.h      # Multi-algorithm benchmark & trade-off analysis
│       ├── CSVManager.h            # Robust CSV parsing, validation, and export
│       ├── TaskQueue.h             # Thread-safe blocking queue (mutex + condition_variable)
│       ├── ThreadPool.h            # Worker thread pool with RAII lifecycle & future return
│       ├── BenchmarkEngine.h       # Concurrency benchmarking & speedup analytics
│       └── CLI.h                   # Interactive menu-driven console application
├── src/
│   ├── Process.cpp                 # Process validation & stream parser
│   ├── ProcessManager.cpp          # Process collection management
│   ├── Scheduler.cpp               # Shared sorting & scheduler utilities
│   ├── FCFS.cpp                    # FCFS scheduling with context-switch handling
│   ├── SJF.cpp                     # SJF scheduling with context-switch handling
│   ├── SRTF.cpp                    # SRTF preemptive scheduling with interval merging
│   ├── RoundRobin.cpp              # Round Robin ready queue & slice management
│   ├── PriorityScheduling.cpp      # Priority scheduling with dynamic aging
│   ├── Metrics.cpp                 # Statistical analysis & fairness computation
│   ├── GanttChart.cpp              # Gantt chart rendering & preemption tracking
│   ├── ComparisonEngine.cpp        # Benchmark execution & trade-off reporting
│   ├── CSVManager.cpp              # CSV reader/writer with strict schema checks
│   ├── TaskQueue.cpp               # TaskQueue explicit template instantiations
│   ├── ThreadPool.cpp              # Worker thread loop & shutdown management
│   ├── BenchmarkEngine.cpp         # CPU-bound benchmarking workload & reporting
│   ├── CLI.cpp                     # Interactive terminal menus, input validation & submenus
│   └── main.cpp                    # CLI entrypoint (interactive, batch, or benchmark flag)
├── samples/                        # Verification workloads
│   ├── processes.csv               # Standard CSV benchmark dataset
│   ├── workload_heavy.csv          # High-volume process batch
│   ├── sample_input.txt            # Text-formatted multi-process benchmark
│   ├── simultaneous_arrival.txt    # Simultaneous arrival benchmark
│   ├── single_process.txt          # Boundary/edge-case benchmark
│   └── with_idle_gaps.txt          # CPU idle-gap handling verification
└── tests/
    └── test_main.cpp               # Comprehensive test suite (203 automated assertions)
```

---

## Simulated Algorithms vs. Real Multithreading

An essential architectural distinction in SchedOS:

1. **CPU Scheduling Engine (Deterministic Simulation)**:
   The FCFS, SJF, SRTF, Round Robin, and Priority algorithms are **mathematical simulations**. They model how an OS kernel schedules processes on a virtual uniprocessor, calculating discrete timelines, idle gaps, preemption boundaries, and context-switching overhead. They do *not* dictate how host OS threads are scheduled on your physical CPU.

2. **Concurrency & Multithreading Engine (Actual Host Execution)**:
   The `ThreadPool` and `TaskQueue` modules spawn and manage **real OS worker threads** (`std::thread`). Tasks submitted to this pool execute concurrently across actual physical CPU cores, utilizing mutual exclusion (`std::mutex`) and condition variables (`std::condition_variable`) for synchronization.

---

## Implemented Scheduling Algorithms

| Algorithm | Type | Preemptive | Aging Support | Criteria / Dispatch Order |
|:---|:---:|:---:|:---:|:---|
| **First-Come, First-Served (FCFS)** | Batch | No | N/A | Arrival Time ($t_{\text{arr}}$) |
| **Shortest Job First (SJF)** | Batch | No | N/A | Burst Time ($t_{\text{burst}}$) among arrived jobs |
| **Shortest Remaining Time First (SRTF)** | Interactive / Real-time | **Yes** | N/A | Remaining Burst Time ($t_{\text{rem}}$) with dynamic preemption |
| **Round Robin (RR)** | Time-sharing | **Yes** | N/A | Configurable Time Quantum ($q$) with FIFO ready queue |
| **Priority Scheduling** | Priority-based | No | **Yes** | Priority value (lower numerical value = higher priority) |

### Priority Aging & Starvation Prevention

In static priority scheduling, low-priority tasks suffer from **starvation (indefinite postponement)** if high-priority tasks arrive continuously. SchedOS implements dynamic priority aging:

$$P_{\text{effective}}(t) = \max\left(0, P_{\text{base}} - \left\lfloor \frac{t - t_{\text{arrival}}}{\text{agingInterval}} \right\rfloor\right)$$

For every `agingInterval` time units spent waiting in the ready queue, the process priority value decreases by 1 (improving its scheduling rank). Once a starved process ages sufficiently, it preempts or precedes newer high-priority processes.

---

## Fairness & Performance Metrics Computed

### Core Process & System Metrics
- **Completion Time ($CT$)**: Timestamp when process completes execution.
- **Turnaround Time ($TAT$)**: Total elapsed time from arrival to completion ($TAT = CT - AT$).
- **Waiting Time ($WT$)**: Total time spent waiting in the ready queue ($WT = TAT - BT$).
- **Response Time ($RT$)**: Duration from process arrival until first CPU allocation ($RT = t_{\text{first}} - AT$).
- **Average Waiting Time**: $\bar{WT} = \frac{1}{N} \sum_{i=1}^N WT_i$
- **Average Turnaround Time**: $\bar{TAT} = \frac{1}{N} \sum_{i=1}^N TAT_i$
- **Average Response Time**: $\bar{RT} = \frac{1}{N} \sum_{i=1}^N RT_i$
- **Productive CPU Utilization**: $\frac{\text{Total Busy Time}}{\text{Total Simulation Duration}} \times 100\%$
- **Total CPU Occupancy**: $\frac{\text{Total Busy Time} + \text{Total Context Switch Overhead}}{\text{Total Simulation Duration}} \times 100\%$
- **Throughput**: $\frac{N}{\text{Total Simulation Duration}}$ processes/unit time

### Fairness & Waiting-Time Dispersion
- **Min / Max Waiting Time**: Identifies the least and most starved processes.
- **Waiting-Time Variance**:
  $$\text{Var}(WT) = \frac{1}{N} \sum_{i=1}^N (WT_i - \bar{WT})^2$$
- **Waiting-Time Standard Deviation**:
  $$\sigma(WT) = \sqrt{\text{Var}(WT)}$$
  *(A lower standard deviation indicates a more equitable, fair distribution of waiting times across all processes).*

---

## Multithreading Architecture & Thread Pool

SchedOS implements an enterprise-grade concurrent execution engine:

### 1. Thread-Safe `TaskQueue<T>`
- **Synchronization**: Thread-safe FIFO queue guarded by `std::mutex` and signaled by `std::condition_variable`.
- **Blocking & Non-Blocking Retrieval**: Provides `pop(T& value)` (blocks until available or queue shutdown) and `tryPop(T& value)` (non-blocking query).
- **Graceful Shutdown**: Thread-safe `shutdown()` notifies all waiting threads to wake and cleanly exit without deadlocks.

### 2. Worker `ThreadPool`
- **Worker Management**: Spawns $N$ worker threads (configurable, defaults to `std::thread::hardware_concurrency()`).
- **RAII Lifecycle**: Thread lifetimes are strictly tied to the pool object. Destructor triggers thread pool shutdown, drains pending tasks, and joins all worker threads. No detached threads; zero thread leaks.
- **Future-Based Task Submission**: The `submit(F&& f, Args&&... args)` template packages callables into `std::packaged_task` and returns `std::future<ReturnType>`, enabling asynchronous result retrieval and transparent exception propagation across thread boundaries.
- **Atomic Task Tracking**: Uses `std::atomic<size_t>` to track submitted and completed task counts lock-free.

---

## Concurrency Performance Benchmarking

The `BenchmarkEngine` evaluates parallel execution efficiency against sequential execution using a repeatable, compute-intensive CPU-bound workload (pseudo-random integer bitwise transformations, cryptographic mixing, and array rotations).

### Methodology
1. **Dead-Code Elimination Prevention**: Task accumulators use `volatile` qualifiers and return checksums so optimizing compilers cannot optimize away work loops.
2. **High-Resolution Clocking**: All timing measured via `std::chrono::steady_clock` to guarantee monotonic time advancement.
3. **Warm-Up Runs & Median Filtering**: Executes warm-up runs to prime CPU caches, followed by multiple benchmark repetitions, reporting the median elapsed duration.
4. **Deterministic Validation**: Verifies that the sequential execution checksum exactly matches the multithreaded checksum ($\text{Checksum}_{\text{seq}} == \text{Checksum}_{\text{par}}$).
5. **Speedup Metric**:
   $$\text{Speedup} = \frac{T_{\text{sequential}}}{T_{\text{parallel}}}$$

### Actual Measured Benchmark Results (Direct Host Hardware)

```
======================================================================
            CONCURRENCY & MULTITHREADING PERFORMANCE BENCHMARK        
======================================================================
  Workload Configuration:
    Total CPU-Bound Tasks     : 100
    Iterations Per Task       : 25,000
    Worker Thread Count       : 16 (Physical / Logical Cores)
    Benchmark Repetitions     : 3 (Median reported)

  Execution Timing Results:
    Sequential Execution Time : 5.53 ms
    Multithreaded Time        : 0.88 ms
    Speedup Ratio             : 6.27x

  Throughput Analysis:
    Sequential Throughput     : 18,070.44 tasks/sec
    Multithreaded Throughput  : 113,314.45 tasks/sec

  Result Validation:
    Sequential Checksum       : 0x35dd8c026717d271
    Parallel Checksum         : 0x35dd8c026717d271
    Equivalence Check         : PASS (Identical Results)
======================================================================
```

### Engineering Analysis (Amdahl's Law & Concurrency Limits)
- **Amdahl's Law**: Maximum attainable speedup is bounded by $\frac{1}{(1 - P) + \frac{P}{N}}$, where $P$ is the parallelizable fraction and $N$ is thread count.
- **Overhead Factors**: Real-world speedup is influenced by thread synchronization, lock contention on the task queue, cache line bouncing, and OS thread context switches.
- **Workload Granularity**: For micro-tasks, synchronization overhead can exceed computation time; for compute-heavy workloads, speedup scales near-linearly with physical core count.

---

## Interactive Command-Line Interface

Launch `./schedos` without arguments for the interactive menu:

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
  11. Concurrency & Multithreading Benchmarking
  12. Exit
--------------------------------------------------------
  Enter your choice (1-12):
```

### Concurrency Submenu (Option 11)
```
========================================================
         Concurrency & Multithreading Submenu           
========================================================
  Current Worker Thread Count: 16
--------------------------------------------------------
  1. Run Multithreaded Workload Demonstration
  2. Configure Worker Thread Count
  3. Benchmark Sequential vs Multithreaded Execution
  4. Display Concurrency Performance Metrics
  5. Run Thread Pool Stress Test (200 concurrent tasks)
  6. Run Dynamic Priority Aging Simulation Demo
  7. Return to Main Menu
--------------------------------------------------------
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

# Run direct concurrency and multithreading benchmark
./schedos -b

# Batch execution on CSV comparing all algorithms
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

The test harness exercises **203 automated assertions** across 18 distinct test suites:

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
--- TaskQueue Tests ---
--- ThreadPool Tests ---
--- BenchmarkEngine Tests ---
--- Fairness & Aging Tests ---
--- Stress & Concurrency Scale Tests ---

========================================
  Results: 203 passed, 0 failed, 203 total
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
- [x] **Phase 3: Multithreading, Concurrency & Performance Benchmarking** (Completed)
  - [x] Thread-safe `TaskQueue` with condition variables & mutex protection
  - [x] Reusable RAII `ThreadPool` with worker threads & `std::future` task submission
  - [x] Performance `BenchmarkEngine` with speedup & throughput analytics
  - [x] Dynamic Priority Aging for starvation prevention
  - [x] Fairness metrics (Waiting-time variance, standard deviation, min/max WT)
  - [x] Interactive Concurrency & Benchmarking CLI Submenu (Option 11) + `-b` flag
  - [x] Stress testing with 100, 1,000, and 10,000 processes + multi-producer thread pool
  - [x] 203 total automated tests passed (0 failures)
