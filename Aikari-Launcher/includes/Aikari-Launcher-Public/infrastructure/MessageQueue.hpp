#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace AikariLauncherPublic::infrastructure::MessageQueue
{
template <typename MessageType>
class SinglePointMessageQueue
{
   public:
    SinglePointMessageQueue() = default;
    ~SinglePointMessageQueue() = default;

    SinglePointMessageQueue(const SinglePointMessageQueue&) = delete;
    SinglePointMessageQueue& operator=(const SinglePointMessageQueue&) = delete;

    void push(MessageType val)
    {
        std::lock_guard<std::mutex> lockGuard(this->lock);
        this->msgQueue.push(val);
        this->cv.notify_one();
        return;
    };

    MessageType pop()
    {
        std::unique_lock<std::mutex> uLock(this->lock);
        this->cv.wait(
            uLock,
            [this]()
            {
                return !this->msgQueue.empty();
            }
        );
        MessageType val = std::move(this->msgQueue.front());
        this->msgQueue.pop();

        return val;
    };

    bool isEmpty() const
    {
        std::lock_guard<std::mutex> lockGuard(this->lock);
        return this->msgQueue.empty();
    };

    size_t getQueueLength() const
    {
        std::lock_guard<std::mutex> lockGuard(this->lock);
        return this->msgQueue.size();
    };

   private:
    mutable std::mutex lock;
    std::condition_variable cv;
    std::queue<MessageType> msgQueue;
};
}  // namespace AikariLauncherPublic::infrastructure::MessageQueue
