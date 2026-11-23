#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

namespace SchedOS {

using Task = std::function<void()>;

/// Thread-safe FIFO task queue using std::mutex and std::condition_variable.
class TaskQueue {
public:
    TaskQueue() = default;
    ~TaskQueue();

    // Non-copyable, non-movable for thread safety
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;

    /// Pushes a task to the queue and notifies one waiting worker.
    void push(Task task);

    /// Blocks until a task becomes available or the queue is stopped.
    /// Returns true if a task was popped, false if the queue was stopped and empty.
    bool pop(Task& task);

    /// Attempts to pop a task without blocking.
    /// Returns true if a task was popped, false otherwise.
    bool tryPop(Task& task);

    /// Stops the queue, wakes all waiting threads, and rejects new pushes.
    void stop();

    /// Returns whether the queue is currently stopped.
    bool isStopped() const;

    /// Returns whether the queue is currently empty.
    bool empty() const;

    /// Returns the current number of queued tasks.
    size_t size() const;

    /// Clears any remaining tasks from the queue.
    void clear();

private:
    std::queue<Task> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stopped_{false};
};

} // namespace SchedOS
