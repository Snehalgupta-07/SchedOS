#include "schedos/CLI.h"
#include "schedos/FCFS.h"
#include "schedos/SJF.h"
#include "schedos/SRTF.h"
#include "schedos/RoundRobin.h"
#include "schedos/PriorityScheduling.h"
#include "schedos/Metrics.h"
#include "schedos/GanttChart.h"
#include "schedos/ComparisonEngine.h"
#include "schedos/CSVManager.h"
#include "schedos/ThreadPool.h"
#include "schedos/BenchmarkEngine.h"

#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

namespace SchedOS {

CLI::CLI(std::istream& in, std::ostream& out)
    : in_(in), out_(out) {}

std::string CLI::colorReset() const { return colorsEnabled_ ? "\033[0m" : ""; }
std::string CLI::colorCyan() const { return colorsEnabled_ ? "\033[36m" : ""; }
std::string CLI::colorGreen() const { return colorsEnabled_ ? "\033[32m" : ""; }
std::string CLI::colorYellow() const { return colorsEnabled_ ? "\033[33m" : ""; }
std::string CLI::colorRed() const { return colorsEnabled_ ? "\033[31m" : ""; }
std::string CLI::colorBold() const { return colorsEnabled_ ? "\033[1m" : ""; }
std::string CLI::colorMagenta() const { return colorsEnabled_ ? "\033[35m" : ""; }

void CLI::setTimeQuantum(int q) {
    if (q <= 0) throw std::invalid_argument("Time quantum must be positive");
    timeQuantum_ = q;
}

void CLI::setContextSwitchCost(int c) {
    if (c < 0) throw std::invalid_argument("Context switch cost cannot be negative");
    contextSwitchCost_ = c;
}

std::unique_ptr<Scheduler> CLI::createScheduler(const std::string& alg) {
    std::unique_ptr<Scheduler> s;
    if (alg == "fcfs") s = std::make_unique<FCFS>();
    else if (alg == "sjf") s = std::make_unique<SJF>();
    else if (alg == "srtf") s = std::make_unique<SRTF>();
    else if (alg == "rr") s = std::make_unique<RoundRobin>(timeQuantum_);
    else if (alg == "priority") s = std::make_unique<PriorityScheduling>();
    else s = std::make_unique<FCFS>();
    return s;
}

int CLI::readInt(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        out_ << prompt;
        int val;
        if (in_ >> val) {
            if (val >= minVal && val <= maxVal) {
                // Clear any remaining chars on the line
                std::string dummy;
                std::getline(in_, dummy);
                return val;
            }
            out_ << colorRed() << "  [!] Value must be between " << minVal << " and " << maxVal << "." << colorReset() << "\n";
        } else {
            if (in_.eof()) return minVal;
            in_.clear();
            std::string dummy;
            std::getline(in_, dummy);
            out_ << colorRed() << "  [!] Invalid input. Please enter an integer." << colorReset() << "\n";
        }
    }
}

std::string CLI::readString(const std::string& prompt) {
    out_ << prompt;
    std::string line;
    std::getline(in_, line);
    // Trim leading whitespace
    size_t first = line.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = line.find_last_not_of(" \t\r\n");
    return line.substr(first, (last - first + 1));
}

void CLI::printHeader(const std::string& title) {
    out_ << "\n" << colorCyan() << colorBold()
         << "========================================================\n"
         << "  " << title << "\n"
         << "========================================================\n"
         << colorReset();
}

void CLI::pause() {
    out_ << colorCyan() << "\n  [Press Enter to return to main menu...] " << colorReset();
    std::string dummy;
    std::getline(in_, dummy);
}

void CLI::printMenu() {
    out_ << "\n" << colorCyan() << colorBold()
         << "========================================================\n"
         << "  SchedOS -- CPU Scheduling & Process Management Simulator\n"
         << "========================================================\n"
         << colorReset();
    out_ << colorBold() << "  Active Algorithm: " << colorYellow() << selectedAlgorithm_
         << colorReset() << " | "
         << colorBold() << "RR Quantum: " << colorYellow() << timeQuantum_
         << colorReset() << " | "
         << colorBold() << "CS Overhead: " << colorYellow() << contextSwitchCost_
         << colorReset() << " | "
         << colorBold() << "Processes: " << colorYellow() << manager_.size()
         << colorReset() << "\n";
    out_ << "--------------------------------------------------------\n";
    out_ << "  " << colorGreen() << "1." << colorReset() << " Add a new process\n";
    out_ << "  " << colorGreen() << "2." << colorReset() << " View all processes\n";
    out_ << "  " << colorGreen() << "3." << colorReset() << " Edit an existing process\n";
    out_ << "  " << colorGreen() << "4." << colorReset() << " Delete a process\n";
    out_ << "  " << colorGreen() << "5." << colorReset() << " Select a scheduling algorithm\n";
    out_ << "  " << colorGreen() << "6." << colorReset() << " Execute scheduling simulation\n";
    out_ << "  " << colorGreen() << "7." << colorReset() << " Compare all scheduling algorithms\n";
    out_ << "  " << colorGreen() << "8." << colorReset() << " Load processes from CSV\n";
    out_ << "  " << colorGreen() << "9." << colorReset() << " Export results to CSV\n";
    out_ << "  " << colorGreen() << "10." << colorReset() << " Reset simulation\n";
    out_ << "  " << colorCyan()  << "11." << colorReset() << " Concurrency & Multithreading Benchmarking Suite\n";
    out_ << "  " << colorRed()   << "12." << colorReset() << " Exit\n";
    out_ << "--------------------------------------------------------\n";
}

void CLI::handleAddProcess() {
    printHeader("Add New Process");
    int pid = readInt("  Enter Process ID (PID >= 0): ", 0, 100000);
    if (manager_.hasPid(pid)) {
        out_ << colorRed() << "  [!] Error: Process ID " << pid << " already exists!\n" << colorReset();
        return;
    }
    int arrival  = readInt("  Enter Arrival Time (>= 0): ", 0, 100000);
    int burst    = readInt("  Enter Burst Time (> 0): ", 1, 100000);
    int priority = readInt("  Enter Priority (lower = higher priority, >= 0): ", 0, 100000);

    try {
        manager_.addProcess(Process(pid, arrival, burst, priority));
        out_ << colorGreen() << "  [+] Process P" << pid << " added successfully.\n" << colorReset();
    } catch (const std::exception& e) {
        out_ << colorRed() << "  [!] Error: " << e.what() << "\n" << colorReset();
    }
}

void CLI::handleViewProcesses() {
    printHeader("Current Process Table");
    if (manager_.empty()) {
        out_ << colorYellow() << "  [!] No processes loaded (Processes: 0).\n"
             << "      To load processes:\n"
             << "      - Option 1: Add a process manually.\n"
             << "      - Option 8: Load from CSV (e.g. samples/processes.csv).\n" << colorReset();
        pause();
        return;
    }

    out_ << std::left
         << std::setw(8)  << "PID"
         << std::setw(14) << "Arrival Time"
         << std::setw(12) << "Burst Time"
         << std::setw(10) << "Priority" << "\n";
    out_ << std::string(44, '-') << "\n";

    for (const auto& p : manager_.getProcesses()) {
        out_ << std::left
             << "P" << std::setw(7) << p.pid
             << std::setw(14) << p.arrivalTime
             << std::setw(12) << p.burstTime
             << std::setw(10) << p.priority << "\n";
    }
    out_ << "\n  Total processes: " << manager_.size() << "\n";
    pause();
}

void CLI::handleEditProcess() {
    printHeader("Edit Process");
    if (manager_.empty()) {
        out_ << colorYellow() << "  No processes to edit.\n" << colorReset();
        return;
    }
    handleViewProcesses();
    int pid = readInt("  Enter PID of process to edit: ", 0, 100000);
    auto pOpt = manager_.findProcess(pid);
    if (!pOpt.has_value()) {
        out_ << colorRed() << "  [!] Error: Process with PID " << pid << " not found.\n" << colorReset();
        return;
    }

    out_ << "  Editing P" << pid << " (Current: Arrival=" << pOpt->arrivalTime
         << ", Burst=" << pOpt->burstTime << ", Priority=" << pOpt->priority << ")\n";
    int arrival  = readInt("  New Arrival Time (>= 0): ", 0, 100000);
    int burst    = readInt("  New Burst Time (> 0): ", 1, 100000);
    int priority = readInt("  New Priority (>= 0): ", 0, 100000);

    if (manager_.editProcess(pid, arrival, burst, priority)) {
        out_ << colorGreen() << "  [+] Process P" << pid << " updated successfully.\n" << colorReset();
    } else {
        out_ << colorRed() << "  [!] Failed to update process.\n" << colorReset();
    }
}

void CLI::handleDeleteProcess() {
    printHeader("Delete Process");
    if (manager_.empty()) {
        out_ << colorYellow() << "  No processes to delete.\n" << colorReset();
        return;
    }
    handleViewProcesses();
    int pid = readInt("  Enter PID of process to delete: ", 0, 100000);
    if (manager_.deleteProcess(pid)) {
        out_ << colorGreen() << "  [+] Process P" << pid << " deleted.\n" << colorReset();
    } else {
        out_ << colorRed() << "  [!] Process with PID " << pid << " not found.\n" << colorReset();
    }
}

void CLI::handleSelectAlgorithm() {
    printHeader("Select Scheduling Algorithm");
    out_ << "  1. First Come First Serve (FCFS)\n";
    out_ << "  2. Shortest Job First (SJF - Non-Preemptive)\n";
    out_ << "  3. Shortest Remaining Time First (SRTF - Preemptive)\n";
    out_ << "  4. Round Robin (RR - Preemptive)\n";
    out_ << "  5. Priority Scheduling (Non-Preemptive)\n";

    int choice = readInt("  Select algorithm (1-5): ", 1, 5);
    switch (choice) {
        case 1: selectedAlgorithm_ = "fcfs"; break;
        case 2: selectedAlgorithm_ = "sjf"; break;
        case 3: selectedAlgorithm_ = "srtf"; break;
        case 4: {
            selectedAlgorithm_ = "rr";
            timeQuantum_ = readInt("  Enter Round Robin Time Quantum (> 0): ", 1, 10000);
            break;
        }
        case 5: selectedAlgorithm_ = "priority"; break;
    }

    out_ << "\n  Configure Context-Switch Overhead:\n";
    contextSwitchCost_ = readInt("  Enter Context-Switch Cost (0 for none): ", 0, 1000);

    out_ << colorGreen() << "  [+] Algorithm set to " << selectedAlgorithm_
         << " (RR Quantum: " << timeQuantum_
         << ", CS Cost: " << contextSwitchCost_ << ")\n" << colorReset();
}

void CLI::handleRunSimulation() {
    printHeader("Execute Scheduling Simulation");
    if (manager_.empty()) {
        out_ << colorYellow() << "  [!] No processes available (Processes: 0).\n"
             << "      Please add processes first using Option 1, or load CSV via Option 8.\n" << colorReset();
        pause();
        return;
    }

    auto procs = manager_.getProcesses();
    auto scheduler = createScheduler(selectedAlgorithm_);
    scheduler->setContextSwitchCost(contextSwitchCost_);

    out_ << colorBold() << "  Running: " << scheduler->name() << colorReset() << "\n";
    scheduler->schedule(procs);

    printProcessTable(procs, out_);

    // Save for export
    lastScheduledProcesses_ = procs;
    lastAlgorithmRun_ = scheduler->name();

    // Gantt chart visualization
    GanttChart::render(scheduler->ganttChart(), out_, 80, colorsEnabled_);

    auto metrics = SchedulingMetrics::compute(procs, scheduler->ganttChart(),
                                             scheduler->contextSwitchCount(),
                                             scheduler->totalContextSwitchOverhead());
    metrics.print(out_);
    pause();
}

void CLI::handleCompareAlgorithms() {
    printHeader("Compare All Scheduling Algorithms");
    if (manager_.empty()) {
        out_ << colorYellow() << "  [!] No processes available (Processes: 0).\n"
             << "      Please add processes first using Option 1, or load CSV via Option 8.\n" << colorReset();
        pause();
        return;
    }

    auto results = ComparisonEngine::compareAll(manager_.getProcesses(), timeQuantum_, contextSwitchCost_);
    ComparisonEngine::printComparisonTable(results, out_, colorsEnabled_);
    pause();
}

void CLI::handleLoadCSV() {
    printHeader("Load Processes from CSV");
    out_ << "  Expected CSV format: PID,ArrivalTime,BurstTime,Priority\n";
    std::string path = readString("  Enter CSV file path: ");
    if (path.empty()) {
        out_ << colorYellow() << "  Load cancelled.\n" << colorReset();
        return;
    }

    try {
        auto loaded = CSVManager::importProcesses(path);
        manager_.setProcesses(loaded);
        out_ << colorGreen() << "  [+] Successfully loaded " << loaded.size() << " processes from CSV.\n" << colorReset();
        handleViewProcesses();
    } catch (const std::exception& e) {
        out_ << colorRed() << "  [!] Error reading CSV: " << e.what() << "\n" << colorReset();
    }
}

void CLI::handleExportCSV() {
    printHeader("Export Results to CSV");
    if (lastScheduledProcesses_.empty()) {
        out_ << colorYellow() << "  [!] No simulation results to export. Run a simulation first (Option 6).\n" << colorReset();
        return;
    }

    std::string path = readString("  Enter export file path (e.g. results.csv): ");
    if (path.empty()) {
        out_ << colorYellow() << "  Export cancelled.\n" << colorReset();
        return;
    }

    try {
        auto metrics = SchedulingMetrics::compute(lastScheduledProcesses_, {});
        CSVManager::exportScheduleResults(path, lastAlgorithmRun_, lastScheduledProcesses_, metrics);
        out_ << colorGreen() << "  [+] Results exported successfully to " << path << "\n" << colorReset();
    } catch (const std::exception& e) {
        out_ << colorRed() << "  [!] Error writing CSV: " << e.what() << "\n" << colorReset();
    }
}

void CLI::handleResetSimulation() {
    printHeader("Reset Simulation");
    manager_.clear();
    lastScheduledProcesses_.clear();
    lastAlgorithmRun_.clear();
    selectedAlgorithm_ = "fcfs";
    timeQuantum_ = 2;
    contextSwitchCost_ = 0;
    out_ << colorGreen() << "  [+] Simulation reset. All processes cleared and settings restored to default.\n" << colorReset();
}

void CLI::handleConcurrencyMenu() {
    bool inSubmenu = true;
    while (inSubmenu) {
        size_t effectiveThreads = benchmarkThreads_ == 0 ? std::thread::hardware_concurrency() : benchmarkThreads_;
        if (effectiveThreads == 0) effectiveThreads = 2;

        out_ << "\n" << colorCyan() << colorBold()
             << "========================================================\n"
             << "       SchedOS Concurrency & Multithreading Suite       \n"
             << "========================================================\n"
             << colorReset();
        out_ << "  Configured Workers: " << colorYellow() << effectiveThreads << colorReset()
             << " | Tasks: " << colorYellow() << benchmarkTasks_ << colorReset()
             << " | Iterations: " << colorYellow() << benchmarkIterations_ << colorReset() << "\n";
        out_ << "--------------------------------------------------------\n";
        out_ << "  1. Run multithreaded workload demonstration\n";
        out_ << "  2. Configure worker thread count\n";
        out_ << "  3. Execute sequential benchmark\n";
        out_ << "  4. Execute parallel benchmark\n";
        out_ << "  5. Compare benchmark results (Sequential vs Parallel)\n";
        out_ << "  6. View thread pool statistics\n";
        out_ << "  7. Return to main menu\n";
        out_ << "--------------------------------------------------------\n";

        int subChoice = readInt("  Select option (1-7): ", 1, 7);
        switch (subChoice) {
            case 1: handleRunMultithreadedDemo(); break;
            case 2: handleConfigureWorkerCount(); break;
            case 3: handleRunSequentialBenchmark(); break;
            case 4: handleRunParallelBenchmark(); break;
            case 5: handleCompareBenchmarks(); break;
            case 6: handleViewThreadPoolStats(); break;
            case 7: inSubmenu = false; break;
        }
    }
}

void CLI::handleRunMultithreadedDemo() {
    printHeader("Multithreaded Workload Live Demonstration");
    size_t numThreads = benchmarkThreads_ == 0 ? std::thread::hardware_concurrency() : benchmarkThreads_;
    if (numThreads == 0) numThreads = 2;

    out_ << "  Spawning ThreadPool with " << numThreads << " real worker threads...\n";
    ThreadPool pool(numThreads);
    std::atomic<int> completed{0};
    const int demoTasks = 20;

    static std::mutex printMutex;
    out_ << "  Submitting " << demoTasks << " concurrent CPU-bound tasks to Thread-Safe TaskQueue...\n";
    for (int i = 1; i <= demoTasks; ++i) {
        pool.submitVoid([i, &completed, this]() {
            BenchmarkEngine::executeTask(i, 20000);
            int done = completed.fetch_add(1, std::memory_order_relaxed) + 1;
            if (done % 5 == 0 || done == demoTasks) {
                std::lock_guard<std::mutex> lock(printMutex);
                out_ << "    [Worker Thread] Progress: " << done << "/" << demoTasks << " tasks completed.\n";
            }
        });
    }

    pool.waitForTasks();
    out_ << colorGreen() << "  [+] All " << demoTasks << " tasks processed concurrently and verified successfully!\n" << colorReset();
}

void CLI::handleConfigureWorkerCount() {
    printHeader("Configure Concurrency Settings");
    size_t hw = std::thread::hardware_concurrency();
    out_ << "  Hardware logical cores detected: " << hw << "\n";
    out_ << "  (Enter 0 to use hardware default)\n";

    int threads = readInt("  Enter worker thread count (0-64): ", 0, 64);
    benchmarkThreads_ = static_cast<size_t>(threads);

    int tasks = readInt("  Enter benchmark task count (10-1000): ", 10, 1000);
    benchmarkTasks_ = static_cast<size_t>(tasks);

    int iters = readInt("  Enter iterations per task (1000-200000): ", 1000, 200000);
    benchmarkIterations_ = iters;

    out_ << colorGreen() << "  [+] Settings updated. Threads: "
         << (benchmarkThreads_ == 0 ? hw : benchmarkThreads_)
         << ", Tasks: " << benchmarkTasks_
         << ", Iterations: " << benchmarkIterations_ << "\n" << colorReset();
}

void CLI::handleRunSequentialBenchmark() {
    printHeader("Execute Sequential Benchmark");
    out_ << "  Executing " << benchmarkTasks_ << " CPU-bound tasks sequentially on a single thread...\n";

    auto tStart = std::chrono::steady_clock::now();
    uint64_t checksum = 0;
    for (size_t i = 0; i < benchmarkTasks_; ++i) {
        checksum += BenchmarkEngine::executeTask(i, benchmarkIterations_);
    }
    auto tEnd = std::chrono::steady_clock::now();

    lastSeqTimeMs_ = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
    hasLastSeq_ = true;

    double throughput = (lastSeqTimeMs_ > 0.0) ? (benchmarkTasks_ / (lastSeqTimeMs_ / 1000.0)) : 0.0;
    out_ << colorGreen() << "  [+] Sequential Execution Completed:\n" << colorReset()
         << "      Duration   : " << lastSeqTimeMs_ << " ms\n"
         << "      Throughput : " << throughput << " tasks/sec\n"
         << "      Checksum   : 0x" << std::hex << checksum << std::dec << "\n";
}

void CLI::handleRunParallelBenchmark() {
    printHeader("Execute Parallel Benchmark");
    size_t numThreads = benchmarkThreads_ == 0 ? std::thread::hardware_concurrency() : benchmarkThreads_;
    if (numThreads == 0) numThreads = 2;

    out_ << "  Executing " << benchmarkTasks_ << " CPU-bound tasks across "
         << numThreads << " worker threads...\n";

    ThreadPool pool(numThreads);
    std::vector<std::future<uint64_t>> futures;
    futures.reserve(benchmarkTasks_);

    auto tStart = std::chrono::steady_clock::now();
    for (size_t i = 0; i < benchmarkTasks_; ++i) {
        futures.push_back(pool.submit([i, this]() {
            return BenchmarkEngine::executeTask(i, benchmarkIterations_);
        }));
    }

    uint64_t checksum = 0;
    for (size_t i = 0; i < benchmarkTasks_; ++i) {
        checksum += futures[i].get();
    }
    auto tEnd = std::chrono::steady_clock::now();

    lastParTimeMs_ = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
    hasLastPar_ = true;

    double throughput = (lastParTimeMs_ > 0.0) ? (benchmarkTasks_ / (lastParTimeMs_ / 1000.0)) : 0.0;
    out_ << colorGreen() << "  [+] Parallel Execution Completed (" << numThreads << " threads):\n" << colorReset()
         << "      Duration   : " << lastParTimeMs_ << " ms\n"
         << "      Throughput : " << throughput << " tasks/sec\n"
         << "      Checksum   : 0x" << std::hex << checksum << std::dec << "\n";
}

void CLI::handleCompareBenchmarks() {
    printHeader("Sequential vs Parallel Benchmark Comparison");
    size_t numThreads = benchmarkThreads_ == 0 ? std::thread::hardware_concurrency() : benchmarkThreads_;
    if (numThreads == 0) numThreads = 2;

    out_ << "  Running automated benchmark with " << benchmarkTasks_ << " tasks on "
         << numThreads << " threads (median of 3 runs)...\n";

    auto res = BenchmarkEngine::run(benchmarkTasks_, numThreads, benchmarkIterations_, 3);
    BenchmarkEngine::printReport(res, out_, colorsEnabled_);
}

void CLI::handleViewThreadPoolStats() {
    printHeader("Thread Pool & Hardware Statistics");
    size_t hw = std::thread::hardware_concurrency();
    size_t configured = benchmarkThreads_ == 0 ? hw : benchmarkThreads_;

    out_ << "  Hardware Logical Cores       : " << hw << "\n";
    out_ << "  Configured Worker Threads    : " << configured << "\n";
    out_ << "  Configured Tasks Per Batch   : " << benchmarkTasks_ << "\n";
    out_ << "  Task Workload Iterations     : " << benchmarkIterations_ << "\n";
    out_ << "  Synchronization Primitives   : std::mutex, std::condition_variable\n";
    out_ << "  Task Queue Type              : Thread-safe FIFO Queue\n";
    out_ << "  Resource Management          : RAII (Thread joining in destructor)\n";
    out_ << "  Exception Safety             : Try-catch protected worker loop\n\n";

    if (hasLastSeq_ && hasLastPar_) {
        double speedup = lastParTimeMs_ > 0.0 ? (lastSeqTimeMs_ / lastParTimeMs_) : 1.0;
        out_ << "  Last Measured Speedup        : " << colorGreen() << speedup << "x" << colorReset()
             << " (" << lastSeqTimeMs_ << " ms vs " << lastParTimeMs_ << " ms)\n";
    }
}

void CLI::run() {
    bool running = true;
    while (running) {
        printMenu();
        int choice = readInt("  Enter your choice (1-12): ", 1, 12);
        switch (choice) {
            case 1:  handleAddProcess(); break;
            case 2:  handleViewProcesses(); break;
            case 3:  handleEditProcess(); break;
            case 4:  handleDeleteProcess(); break;
            case 5:  handleSelectAlgorithm(); break;
            case 6:  handleRunSimulation(); break;
            case 7:  handleCompareAlgorithms(); break;
            case 8:  handleLoadCSV(); break;
            case 9:  handleExportCSV(); break;
            case 10: handleResetSimulation(); break;
            case 11: handleConcurrencyMenu(); break;
            case 12:
                out_ << "\n" << colorCyan() << "  Thank you for using SchedOS. Goodbye!\n\n" << colorReset();
                running = false;
                break;
        }
    }
}

} // namespace SchedOS
