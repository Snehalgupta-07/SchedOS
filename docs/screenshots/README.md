# SchedOS — Terminal Execution Visualizations & Reproducibility Guide

This directory contains genuine captured terminal sessions from **SchedOS**. Every artifact is directly reproducible using the exact commands provided below.

---

## 1. Interactive Main Menu
- **Captured File**: [`01_main_menu.txt`](01_main_menu.txt)
- **Command to Reproduce**:
  ```powershell
  .\build\schedos.exe
  ```
- **Terminal Preview**:
  ```
  ========================================================
    SchedOS — CPU Scheduling & Process Management Simulator
  ========================================================
    Active Algorithm: fcfs | RR Quantum: 2 | CS Overhead: 0 | Processes: 0
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
    11. Concurrency & Multithreading Benchmarking Suite
    12. Exit
  --------------------------------------------------------
    Enter your choice (1-12):
  ```

---

## 2. In-Memory Process Management & Table View
- **Captured File**: [`02_process_management.txt`](02_process_management.txt)
- **Command to Reproduce**:
  ```powershell
  # Launch SchedOS, select Option 8 (Load CSV), enter "samples/processes.csv", select Option 2 (View)
  .\build\schedos.exe
  ```
- **Terminal Preview**:
  ```
  ========================================================
    Current Process Table
  ========================================================
  PID     Arrival Time  Burst Time  Priority  
  --------------------------------------------
  P1      0             8           2         
  P2      1             4           1         
  P3      2             9           3         
  P4      3             5           4         
  P5      4             2           0         

    Total processes: 5
  ```

---

## 3. Gantt Chart Terminal Visualization with Context Switching
- **Captured File**: [`03_gantt_chart.txt`](03_gantt_chart.txt)
- **Command to Reproduce**:
  ```powershell
  .\build\schedos.exe -f samples/processes.csv -a rr -q 2 -c 1
  ```
- **Terminal Preview**:
  ```
  === Gantt Chart Visualization ===

    +------+----+------+----+------+----+------+----+------+----+------+----+
    |  P1  | CS |  P2  | CS |  P3  | CS |  P1  | CS |  P4  | CS |  P5  | CS |
    +------+----+------+----+------+----+------+----+------+----+------+----+
    0      2    3      5    6      8    9      11   12     14   15     17   18

    +------+----+------+----+------+----+------+----+------+----+------+----+----+
    |  P2  | CS |  P3  | CS |  P1  | CS |  P4  | CS |  P3  | CS |  P1  | CS | P4 |
    +------+----+------+----+------+----+------+----+------+----+------+----+----+
    18     20   21     23   24     26   27     29   30     32   33     35   36   37

    +----+--------+
    | CS |   P3   |
    +----+--------+
    37   38       41

    Timeline: P1 [0-2] -> CS [2-3] -> P2 [3-5] -> CS [5-6] -> P3 [6-8] -> CS [8-9] -> P1 [9-11] -> CS [11-12] -> P4 [12-14] -> CS [14-15] -> P5 [15-17] -> CS [17-18] -> P2 [18-20] -> CS [20-21] -> P3 [21-23] -> CS [23-24] -> P1 [24-26] -> CS [26-27] -> P4 [27-29] -> CS [29-30] -> P3 [30-32] -> CS [32-33] -> P1 [33-35] -> CS [35-36] -> P4 [36-37] -> CS [37-38] -> P3 [38-41]

    Execution Slices per Process:
      P1: [0 -> 2], [9 -> 11], [24 -> 26], [33 -> 35] (Active Time: 8, Slices: 4, Preemptions: 3)
      P2: [3 -> 5], [18 -> 20] (Active Time: 4, Slices: 2, Preemptions: 1)
      P3: [6 -> 8], [21 -> 23], [30 -> 32], [38 -> 41] (Active Time: 9, Slices: 4, Preemptions: 3)
      P4: [12 -> 14], [27 -> 29], [36 -> 37] (Active Time: 5, Slices: 3, Preemptions: 2)
      P5: [15 -> 17] (Active Time: 2, Slices: 1)
      Context Switching: Total Overhead = 13 units
  ```

---

## 4. Multi-Algorithm Performance Comparison Benchmark
- **Captured File**: [`04_algorithm_comparison.txt`](04_algorithm_comparison.txt)
- **Command to Reproduce**:
  ```powershell
  # Option 7 from within SchedOS after loading processes.csv
  .\build\schedos.exe -f samples/processes.csv -a all
  ```
- **Terminal Preview**:
  ```
  ====================================================================================================
                               ALGORITHM PERFORMANCE COMPARISON BENCHMARK                             
  ====================================================================================================
  Algorithm                             Avg WT    Avg TAT   Avg RT    Prod. CPU(%)  Throughput    CS Count  Total Time
  ----------------------------------------------------------------------------------------------------------
  First Come First Serve (FCFS)         11.40     17.00     11.40     100.0%*       0.1785*       0*        28*       
  Shortest Job First (SJF)              8.20      13.80     8.20      100.0%*       0.1785*       0*        28*       
  Shortest Remaining Time First (SRTF)  6.600*    12.20*    4.40      100.0%*       0.1785*       0*        28*       
  Round Robin (RR)                      13.00     18.60     2.800*    100.0%*       0.1785*       0*        28*       
  Priority Scheduling (Non-Preemptive)  9.00      14.60     9.00      100.0%*       0.1785*       0*        28*       
  ----------------------------------------------------------------------------------------------------
    * Indicates best performance for that metric.
  ```

---

## 5. Concurrency & Multithreading Benchmark
- **Captured File**: [`05_multithreading_benchmark.txt`](05_multithreading_benchmark.txt)
- **Command to Reproduce**:
  ```powershell
  .\build\schedos.exe -b
  ```
- **Terminal Preview**:
  ```
  ======================================================================
              CONCURRENCY & MULTITHREADING PERFORMANCE BENCHMARK        
  ======================================================================
    Workload Configuration:
      Total CPU-Bound Tasks     : 100
      Iterations Per Task       : 25000
      Worker Thread Count       : 16
      Benchmark Repetitions     : 3 (Median reported)

    Execution Timing Results:
      Sequential Execution Time : 5.53 ms
      Multithreaded Time        : 0.88 ms
      Speedup Ratio             : 6.27x

    Throughput Analysis:
      Sequential Throughput     : 18070.44 tasks/sec
      Multithreaded Throughput  : 113314.45 tasks/sec

    Result Validation:
      Sequential Checksum       : 0x35dd8c026717d271
      Parallel Checksum         : 0x35dd8c026717d271
      Equivalence Check         : PASS (Identical Results)
  ======================================================================
  ```
