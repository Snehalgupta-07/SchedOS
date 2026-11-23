#include "schedos/ThreadPool.h"
#include <algorithm>

namespace SchedOS {

ThreadPool::ThreadPool(size_t numThreads) {
    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) numThreads = 2; // Fallback
    }

    workers_.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back(&ThreadPool::workerLoop, this, i);
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}

void ThreadPool::submitVoid(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        if (shutdown_.load(std::memory_order_relaxed)) {
            throw std::runtime_error("ThreadPool::submitVoid called on shut down ThreadPool");
        }
        totalTasksSubmitted_.fetch_add(1, std::memory_order_relaxed);
        tasks_.push(std::move(task));
    }
    cvTaskAvailable_.notify_one();
}

void ThreadPool::waitForTasks() {
    std::unique_lock<std::mutex> lock(queueMutex_);
    cvTasksDone_.wait(lock, [this]() {
        return tasks_.empty() && (activeWorkers_.load(std::memory_order_relaxed) == 0);
    });
}

void ThreadPool::shutdown() {
    bool expected = false;
    if (shutdown_.compare_exchange_strong(expected, true)) {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
        }
        cvTaskAvailable_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
}

size_t ThreadPool::pendingTaskCount() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return tasks_.size();
}

void ThreadPool::workerLoop(size_t workerId) {
    (void)workerId;
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            cvTaskAvailable_.wait(lock, [this]() {
                return shutdown_.load(std::memory_order_relaxed) || !tasks_.empty();
            });

            if (shutdown_.load(std::memory_order_relaxed) && tasks_.empty()) {
                return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
            activeWorkers_.fetch_add(1, std::memory_order_relaxed);
        }

        try {
            task();
        } catch (...) {
            // Guard worker thread against task exceptions
        }

        activeWorkers_.fetch_sub(1, std::memory_order_relaxed);
        completedTasks_.fetch_add(1, std::memory_order_relaxed);

        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if (tasks_.empty() && (activeWorkers_.load(std::memory_order_relaxed) == 0)) {
                cvTasksDone_.notify_all();
            }
        }
    }
}

} // namespace SchedOS
