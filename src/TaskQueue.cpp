#include "schedos/TaskQueue.h"

namespace SchedOS {

TaskQueue::~TaskQueue() {
    stop();
}

void TaskQueue::push(Task task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopped_.load(std::memory_order_relaxed)) {
            return; // Ignore pushes after stop
        }
        queue_.push(std::move(task));
    }
    cv_.notify_one();
}

bool TaskQueue::pop(Task& task) {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() {
        return !queue_.empty() || stopped_.load(std::memory_order_relaxed);
    });

    if (queue_.empty()) {
        return false; // Stopped and queue is drained
    }

    task = std::move(queue_.front());
    queue_.pop();
    return true;
}

bool TaskQueue::tryPop(Task& task) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
        return false;
    }
    task = std::move(queue_.front());
    queue_.pop();
    return true;
}

void TaskQueue::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_.store(true, std::memory_order_relaxed);
    }
    cv_.notify_all();
}

bool TaskQueue::isStopped() const {
    return stopped_.load(std::memory_order_relaxed);
}

bool TaskQueue::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

size_t TaskQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

void TaskQueue::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::queue<Task> emptyQueue;
    std::swap(queue_, emptyQueue);
}

} // namespace SchedOS
