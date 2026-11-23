#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>
#include <memory>
#include <type_traits>

namespace SchedOS {

/// Industrial-grade, reusable C++17 thread pool.
/// Manages a group of worker threads executing tasks concurrently.
class ThreadPool {
public:
    /// Constructs a thread pool with numThreads workers.
    /// If numThreads == 0, defaults to std::thread::hardware_concurrency() (min 1).
    explicit ThreadPool(size_t numThreads = 0);

    /// Destructor automatically triggers graceful shutdown and joins all workers.
    ~ThreadPool();

    // Non-copyable, non-movable (RAII resource manager)
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /// Submits a callable task and returns a std::future holding its result.
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();

        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if (shutdown_.load(std::memory_order_relaxed)) {
                throw std::runtime_error("ThreadPool::submit called on shut down ThreadPool");
            }
            totalTasksSubmitted_.fetch_add(1, std::memory_order_relaxed);
            tasks_.push([task]() {
                (*task)();
            });
        }
        cvTaskAvailable_.notify_one();
        return res;
    }

    /// Submits a fire-and-forget task without a future.
    void submitVoid(std::function<void()> task);

    /// Blocks calling thread until all queued and executing tasks are finished.
    void waitForTasks();

    /// Initiates graceful shutdown and joins all worker threads.
    void shutdown();

    /// Returns the number of worker threads in the pool.
    size_t workerCount() const { return workers_.size(); }

    /// Returns the number of tasks currently waiting in the queue.
    size_t pendingTaskCount() const;

    /// Returns the total number of tasks completed since creation.
    size_t completedTaskCount() const { return completedTasks_.load(std::memory_order_relaxed); }

    /// Returns the total number of tasks submitted.
    size_t submittedTaskCount() const { return totalTasksSubmitted_.load(std::memory_order_relaxed); }

    /// Returns whether the thread pool is shut down.
    bool isShutdown() const { return shutdown_.load(std::memory_order_relaxed); }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    mutable std::mutex queueMutex_;
    std::condition_variable cvTaskAvailable_;
    std::condition_variable cvTasksDone_;

    std::atomic<bool> shutdown_{false};
    std::atomic<size_t> activeWorkers_{0};
    std::atomic<size_t> completedTasks_{0};
    std::atomic<size_t> totalTasksSubmitted_{0};

    void workerLoop(size_t workerId);
};

} // namespace SchedOS
