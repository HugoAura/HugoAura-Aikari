#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace AikariShared::infrastructure::MessageQueue
{
    template <typename TaskType>
    class PoolQueue
    {
       public:
        PoolQueue(size_t threads, std::function<void(TaskType)> execFunc)
            : execFunction(execFunc)
        {
            for (size_t i = 0; i < threads; i++)
            {
                this->workers.emplace_back(&PoolQueue::workerLoop, this);
            }
        };

        ~PoolQueue()
        {
            {
                std::unique_lock<std::mutex> lock(this->taskMutex);
                this->isRunning = false;
            }
            this->cv.notify_all();
            for (auto& perThread : this->workers)
            {
                perThread.join();
            }
        };

        PoolQueue(const PoolQueue&) = delete;
        PoolQueue& operator=(const PoolQueue&) = delete;

        void pushTask(TaskType& newTask)
        {
            {
                std::unique_lock<std::mutex> uLock(this->taskMutex);
                this->tasks.emplace(newTask);
            }
            cv.notify_one();
        }

        void insertTask(TaskType newTask)
        {
            {
                std::unique_lock<std::mutex> uLock(this->taskMutex);
                this->tasks.emplace(std::move(newTask));
            }
            cv.notify_one();
        }

       private:
        std::vector<std::jthread> workers;
        std::queue<TaskType> tasks;
        std::function<void(TaskType)> execFunction;

        std::mutex taskMutex;
        std::condition_variable cv;
        bool isRunning = true;

        void workerLoop()
        {
            while (true)
            {
                std::unique_lock<std::mutex> uLock(this->taskMutex);
                cv.wait(
                    uLock,
                    [this]
                    {
                        return !this->isRunning || !this->tasks.empty();
                    }
                );

                if (!this->isRunning)
                {
                    return;
                }

                TaskType curTask = std::move(this->tasks.front());
                this->tasks.pop();
                uLock.unlock();
                this->execFunction(std::move(curTask));
            }
        }
    };
}  // namespace AikariShared::infrastructure::MessageQueue
