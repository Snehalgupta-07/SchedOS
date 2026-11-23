#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <cstdlib>

#include "schedos/Process.h"
#include "schedos/Scheduler.h"
#include "schedos/FCFS.h"
#include "schedos/SJF.h"
#include "schedos/SRTF.h"
#include "schedos/RoundRobin.h"
#include "schedos/PriorityScheduling.h"
#include "schedos/Metrics.h"
#include "schedos/CLI.h"
#include "schedos/GanttChart.h"
#include "schedos/CSVManager.h"
#include "schedos/ComparisonEngine.h"

using namespace SchedOS;

void printGanttChart(const std::vector<GanttEntry>& gantt, std::ostream& out = std::cout) {
    out << "Gantt Chart: ";
    for (const auto& [pid, start, end] : gantt) {
        if (pid == -1)
            out << "[IDLE:" << start << "-" << end << "] ";
        else if (pid == -2)
            out << "[CS:" << start << "-" << end << "] ";
        else
            out << "[P" << pid << ":" << start << "-" << end << "] ";
    }
    out << "\n";
}

void printUsage(const char* progName) {
    std::cout << "\nSchedOS — CPU Scheduling and Process Management Simulator\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << progName << "                  Launch interactive menu-driven CLI\n";
    std::cout << "  " << progName << " [options]        Execute in batch mode\n\n";
    std::cout << "Options:\n";
    std::cout << "  -f <file>       Input file with process data (text or CSV)\n";
    std::cout << "  -a <algorithm>  Algorithm: fcfs, sjf, srtf, rr, priority, all (default: all)\n";
    std::cout << "  -q <quantum>    Time quantum for Round Robin (default: 2)\n";
    std::cout << "  -c <cost>       Context switch cost (default: 0)\n";
    std::cout << "  -h              Show this help message\n\n";
}

void runAlgorithm(Scheduler& scheduler, std::vector<Process> processes, int csCost = 0) {
    std::cout << "\n========================================\n";
    std::cout << "  " << scheduler.name() << "\n";
    std::cout << "========================================\n";

    scheduler.setContextSwitchCost(csCost);
    scheduler.schedule(processes);

    printProcessTable(processes, std::cout);
    GanttChart::render(scheduler.ganttChart(), std::cout, 80, true);

    auto metrics = SchedulingMetrics::compute(processes, scheduler.ganttChart());
    metrics.print(std::cout);
}

int main(int argc, char* argv[]) {
    // If no arguments, launch the interactive CLI
    if (argc == 1) {
        CLI cli;
        cli.run();
        return 0;
    }

    std::string inputFile;
    std::string algorithm = "all";
    int quantum = 2;
    int csCost = 0;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-f" && i + 1 < argc) {
            inputFile = argv[++i];
        } else if (arg == "-a" && i + 1 < argc) {
            algorithm = argv[++i];
        } else if (arg == "-q" && i + 1 < argc) {
            quantum = std::atoi(argv[++i]);
        } else if (arg == "-c" && i + 1 < argc) {
            csCost = std::atoi(argv[++i]);
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    std::vector<Process> processes;
    if (!inputFile.empty()) {
        if (inputFile.length() >= 4 && inputFile.substr(inputFile.length() - 4) == ".csv") {
            try {
                processes = CSVManager::importProcesses(inputFile);
            } catch (const std::exception& e) {
                std::cerr << "Error loading CSV: " << e.what() << "\n";
                return 1;
            }
        } else {
            std::ifstream fin(inputFile);
            if (!fin.is_open()) {
                std::cerr << "Error: cannot open file '" << inputFile << "'\n";
                return 1;
            }
            processes = readProcesses(fin);
        }
    } else {
        std::cout << "Enter processes (PID ArrivalTime BurstTime Priority), one per line.\n";
        std::cout << "Press Ctrl+Z (Windows) or Ctrl+D (Unix) to finish:\n\n";
        processes = readProcesses(std::cin);
    }

    if (processes.empty()) {
        std::cerr << "Error: no valid processes provided.\n";
        return 1;
    }

    std::cout << "\nLoaded " << processes.size() << " process(es).\n";

    auto run = [&](const std::string& alg) {
        if (alg == "fcfs" || alg == "all") {
            FCFS fcfs;
            runAlgorithm(fcfs, processes, csCost);
        }
        if (alg == "sjf" || alg == "all") {
            SJF sjf;
            runAlgorithm(sjf, processes, csCost);
        }
        if (alg == "srtf" || alg == "all") {
            SRTF srtf;
            runAlgorithm(srtf, processes, csCost);
        }
        if (alg == "rr" || alg == "all") {
            RoundRobin rr(quantum);
            runAlgorithm(rr, processes, csCost);
        }
        if (alg == "priority" || alg == "all") {
            PriorityScheduling ps;
            runAlgorithm(ps, processes, csCost);
        }
        if (alg != "fcfs" && alg != "sjf" && alg != "srtf" &&
            alg != "rr" && alg != "priority" && alg != "all") {
            std::cerr << "Unknown algorithm: " << alg << "\n";
            std::cerr << "Available: fcfs, sjf, srtf, rr, priority, all\n";
        }
    };

    if (algorithm == "compare") {
        auto results = ComparisonEngine::compareAll(processes, quantum, csCost);
        ComparisonEngine::printComparisonTable(results, std::cout, true);
        return 0;
    }

    run(algorithm);

    return 0;
}
