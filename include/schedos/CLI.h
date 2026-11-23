#pragma once

#include "schedos/ProcessManager.h"
#include "schedos/Scheduler.h"
#include "schedos/BenchmarkEngine.h"
#include <iostream>
#include <memory>
#include <string>

namespace SchedOS {

class CLI {
public:
    CLI(std::istream& in = std::cin, std::ostream& out = std::cout);

    /// Starts the interactive menu loop.
    void run();

    /// Process management handlers
    void handleAddProcess();
    void handleViewProcesses();
    void handleEditProcess();
    void handleDeleteProcess();
    void handleSelectAlgorithm();
    void handleRunSimulation();
    void handleCompareAlgorithms();
    void handleLoadCSV();
    void handleExportCSV();
    void handleResetSimulation();

    /// Concurrency & Multithreading Handlers
    void handleConcurrencyMenu();
    void handleRunMultithreadedDemo();
    void handleConfigureWorkerCount();
    void handleRunSequentialBenchmark();
    void handleRunParallelBenchmark();
    void handleCompareBenchmarks();
    void handleViewThreadPoolStats();

    // Configuration
    void setTimeQuantum(int q);
    int getTimeQuantum() const { return timeQuantum_; }
    void setContextSwitchCost(int c);
    int getContextSwitchCost() const { return contextSwitchCost_; }
    void setSelectedAlgorithm(const std::string& alg) { selectedAlgorithm_ = alg; }
    const std::string& getSelectedAlgorithm() const { return selectedAlgorithm_; }

    void setWorkerCount(size_t threads) { benchmarkThreads_ = threads; }
    size_t getWorkerCount() const { return benchmarkThreads_; }

    ProcessManager& getProcessManager() { return manager_; }
    const ProcessManager& getProcessManager() const { return manager_; }

    void enableColors(bool enabled) { colorsEnabled_ = enabled; }
    bool colorsEnabled() const { return colorsEnabled_; }

private:
    std::istream& in_;
    std::ostream& out_;
    ProcessManager manager_;

    std::string selectedAlgorithm_ = "fcfs";
    int timeQuantum_ = 2;
    int contextSwitchCost_ = 0;
    bool colorsEnabled_ = true;

    // Concurrency benchmark configuration
    size_t benchmarkThreads_ = 0;
    size_t benchmarkTasks_ = 100;
    int benchmarkIterations_ = 25000;
    double lastSeqTimeMs_ = 0.0;
    double lastParTimeMs_ = 0.0;
    bool hasLastSeq_ = false;
    bool hasLastPar_ = false;

    // Last simulation results for export
    std::vector<Process> lastScheduledProcesses_;
    std::string lastAlgorithmRun_;

    void printMenu();
    void printHeader(const std::string& title);
    int readInt(const std::string& prompt, int minVal = 0, int maxVal = 1000000);
    std::string readString(const std::string& prompt);

    // ANSI Color helpers
    std::string colorReset() const;
    std::string colorCyan() const;
    std::string colorGreen() const;
    std::string colorYellow() const;
    std::string colorRed() const;
    std::string colorBold() const;
    std::string colorMagenta() const;

    std::unique_ptr<Scheduler> createScheduler(const std::string& alg);
};

} // namespace SchedOS
