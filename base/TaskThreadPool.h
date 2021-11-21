#ifndef TASKTHREADPOOL_H
#define TASKTHREADPOOL_H

#include "Threads.h"
#include "RecycleQueue.h"
#include "RecycleQueueBlockThreadSafeTwo.h"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <string>
#include <vector>
#include <functional>

namespace sunflower
{
    class TaskThreadPool : public Noncopyable
    {
    public:
        using task_t = std::function<void()>;
        explicit TaskThreadPool(uint32_t workerNum = 1, uint32_t queuePow = 20);
        ~TaskThreadPool();

        void Start();
        void Stop();
        void SetWorkerNum(uint32_t num);
        void WaitPushTask(task_t &&task) { _pendingTask.waitPush(std::move(task)); }
        bool TryPushTask(task_t &&task) { return _pendingTask.tryPush(std::move(task)); }
        uint32_t GetWorkerNum() const { return _vecWorker.size(); }
        uint32_t GetTaskNum() const { return _pendingTask.size(); }

    private:
        class Worker : public Thread
        {
        public:
            Worker(TaskThreadPool *parent, uint32_t workerId);
            ~Worker() {}
            uint32_t GetWorkerId() const { return _workerId; }
            bool IsStarted() const { return _started; }
            void Stop();

        private:
            void RunThread() override;
            bool WaitTask(task_t *task);

        private:
            TaskThreadPool *_parent = nullptr;
            uint32_t _workerId = 0;
            bool _started = false;
        };

    private:
        uint32_t _workerNum = 0;
        std::vector<std::unique_ptr<Worker>> _vecWorker;
        std::mutex _poolMutex = {};
        bool _started = false;
        // queue
        RecycleQueueBlockThreadSafeTwo<task_t> _pendingTask;
    };

    class TaskThreadPoolQueues : public Noncopyable
    {
    public:
        using task_t = std::function<void()>;
        explicit TaskThreadPoolQueues(uint32_t workerNum = 1, uint32_t queuePow = 20);
        ~TaskThreadPoolQueues();

        void Start();
        void Stop();
        void SetWorkerNum(uint32_t num);
        bool TryPushTask(task_t &&task);
        uint32_t GetWorkerNum() const { return _vecWorker.size(); }
        uint32_t GetTaskNum();

    private:
        class Worker : public Thread
        {
        public:
            Worker(TaskThreadPoolQueues *parent, uint32_t workerId, uint32_t queuePow);
            ~Worker() {}
            uint32_t GetWorkerId() const { return _workerId; }
            bool IsStarted() const { return _started; }
            void Stop();

        private:
            void RunThread() override;
            bool WaitTask(task_t *task);

        public:
            // queue
            RecycleQueue<task_t> _pendingTask;

        private:
            TaskThreadPoolQueues *_parent = nullptr;
            uint32_t _workerId = 0;
            bool _started = false;
        };

    private:
        uint32_t _workerNum = 0;
        uint32_t _queuePow = 0;
        uint32_t _lastPos = 0;
        std::vector<std::unique_ptr<Worker>> _vecWorker;
        std::mutex _poolMutex = {};
        bool _started = false;
    };
} // namespace sunflower

#endif //TASK_THREAD_POOL_H
