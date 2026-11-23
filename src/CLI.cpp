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

void CLI::printMenu() {
    out_ << "\n" << colorCyan() << colorBold()
         << "========================================================\n"
         << "  SchedOS — CPU Scheduling & Process Management Simulator\n"
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
    out_ << "  " << colorRed()   << "11." << colorReset() << " Exit\n";
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
        out_ << colorYellow() << "  No processes loaded. Use Option 1 to add or Option 8 to load from CSV.\n" << colorReset();
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
        out_ << colorYellow() << "  [!] No processes available. Please add processes first.\n" << colorReset();
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

    auto metrics = SchedulingMetrics::compute(procs, scheduler->ganttChart());
    metrics.print(out_);
}

void CLI::handleCompareAlgorithms() {
    printHeader("Compare All Scheduling Algorithms");
    if (manager_.empty()) {
        out_ << colorYellow() << "  [!] No processes available. Please add processes first.\n" << colorReset();
        return;
    }

    auto results = ComparisonEngine::compareAll(manager_.getProcesses(), timeQuantum_, contextSwitchCost_);
    ComparisonEngine::printComparisonTable(results, out_, colorsEnabled_);
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

void CLI::run() {
    bool running = true;
    while (running) {
        printMenu();
        int choice = readInt("  Enter your choice (1-11): ", 1, 11);
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
            case 11:
                out_ << "\n" << colorCyan() << "  Thank you for using SchedOS. Goodbye!\n\n" << colorReset();
                running = false;
                break;
        }
    }
}

} // namespace SchedOS
