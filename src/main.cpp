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

using namespace SchedOS;

void printGanttChart(const std::vector<GanttEntry>& gantt, std::ostream& out = std::cout) {
    out << "Gantt Chart: ";
    for (const auto& [pid, start, end] : gantt) {
        if (pid == -1)
            out << "[IDLE:" << start << "-" << end << "] ";
        else
            out << "[P" << pid << ":" << start << "-" << end << "] ";
    }
    out << "\n";
}

void printUsage(const char* progName) {
    std::cout << "\nSchedOS — CPU Scheduling and Process Management Simulator\n\n";
    std::cout << "Usage: " << progName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -f <file>       Input file with process data\n";
    std::cout << "  -a <algorithm>  Algorithm: fcfs, sjf, srtf, rr, priority, all\n";
    std::cout << "  -q <quantum>    Time quantum for Round Robin (default: 2)\n";
    std::cout << "  -h              Show this help message\n\n";
    std::cout << "Input format (one process per line):\n";
    std::cout << "  PID  ArrivalTime  BurstTime  Priority\n\n";
    std::cout << "Example:\n";
    std::cout << "  1 0 10 2\n";
    std::cout << "  2 1 5  1\n";
    std::cout << "  3 2 8  3\n\n";
}

void runAlgorithm(Scheduler& scheduler, std::vector<Process> processes) {
    std::cout << "\n========================================\n";
    std::cout << "  " << scheduler.name() << "\n";
    std::cout << "========================================\n";

    scheduler.schedule(processes);

    printProcessTable(processes, std::cout);
    printGanttChart(scheduler.ganttChart());

    auto metrics = SchedulingMetrics::compute(processes, scheduler.ganttChart());
    metrics.print(std::cout);
}

int main(int argc, char* argv[]) {
    std::string inputFile;
    std::string algorithm = "all";
    int quantum = 2;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-f" && i + 1 < argc) {
            inputFile = argv[++i];
        } else if (arg == "-a" && i + 1 < argc) {
            algorithm = argv[++i];
        } else if (arg == "-q" && i + 1 < argc) {
            quantum = std::atoi(argv[++i]);
        } else if (arg == "-h") {
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
        std::ifstream fin(inputFile);
        if (!fin.is_open()) {
            std::cerr << "Error: cannot open file '" << inputFile << "'\n";
            return 1;
        }
        processes = readProcesses(fin);
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
            runAlgorithm(fcfs, processes);
        }
        if (alg == "sjf" || alg == "all") {
            SJF sjf;
            runAlgorithm(sjf, processes);
        }
        if (alg == "srtf" || alg == "all") {
            SRTF srtf;
            runAlgorithm(srtf, processes);
        }
        if (alg == "rr" || alg == "all") {
            RoundRobin rr(quantum);
            runAlgorithm(rr, processes);
        }
        if (alg == "priority" || alg == "all") {
            PriorityScheduling ps;
            runAlgorithm(ps, processes);
        }
        if (alg != "fcfs" && alg != "sjf" && alg != "srtf" &&
            alg != "rr" && alg != "priority" && alg != "all") {
            std::cerr << "Unknown algorithm: " << alg << "\n";
            std::cerr << "Available: fcfs, sjf, srtf, rr, priority, all\n";
        }
    };

    run(algorithm);

    return 0;
}
