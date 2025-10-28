#pragma once
#include <tbb/concurrent_queue.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <condition_variable>

enum class RemoveQueueResult
{
    Success,
    NotFound
};

// TBB package version in vcpkg repo is low
template <typename T>
class BlockingQueue
{
public:
    void push(const T &item)
    {
        queue_.push(item);
        cv_.notify_one();
    }

    // blocking pop
    T pop()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]
                 { return !queue_.empty(); });
        T item;
        queue_.try_pop(item);
        return item;
    }

    // non-blocking pop
    bool try_pop(T &item)
    {
        return queue_.try_pop(item);
    }

    bool empty() const
    {
        return queue_.empty();
    }

private:
    tbb::concurrent_queue<T> queue_;
    mutable std::mutex mtx_;
    std::condition_variable cv_;
};

// QueueRegistry using BlockingQueue
template <typename T>
class QueueRegistry
{
public:
    // Singleton instance
    static QueueRegistry &instance()
    {
        static QueueRegistry inst;
        return inst;
    }

    // Get or create a queue by name
    std::shared_ptr<BlockingQueue<T>> get_queue(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = queues_.find(name);
        if (it == queues_.end())
        {
            auto q = std::make_shared<BlockingQueue<T>>();
            queues_[name] = q;
            return q;
        }
        return it->second;
    }

    RemoveQueueResult remove_queue(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = queues_.find(name);
        if (it != queues_.end())
        {
            queues_.erase(it);
            return RemoveQueueResult::Success;
        }
        return RemoveQueueResult::NotFound;
    }

private:
    QueueRegistry() = default;
    std::unordered_map<std::string, std::shared_ptr<BlockingQueue<T>>> queues_;
    std::mutex mutex_;
};
