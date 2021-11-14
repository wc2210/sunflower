#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "Threads.h"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <string>
#include <vector>

namespace sunflower
{
    enum class Status : unsigned
    {
        NONE = 0,
        IDLE,
        READY,
        START,
        WORKING
    };

    class ThreadPool
    {
    public:
        class Worker : public Thread
        {
        public:
            Worker() : Thread() {}
            ~Worker() {}
            void Init(uint32_t workerId, ThreadPool *parent);
            uint32_t WorkerId() const { return _workerId; }
            void Finished();

        protected:
            virtual void RunWorker(void *arg) = 0;

        private:
            void RunThread() override;

        private:
            uint32_t _workerId = 0;
            ThreadPool *_parent = nullptr;
            std::mutex *_parentMutex = nullptr;
            std::condition_variable *_parentCondition = nullptr;
            std::condition_variable *_condition = nullptr;
            Status *_status = nullptr;
            void **_arg = nullptr;
        };

    public:
        explicit ThreadPool(int workerCapacity = -1);
        ~ThreadPool();
        int RegisterWorker(Worker *worker);
        bool DoTask(void *arg, bool block = true);
        std::pair<Worker *, uint32_t> WaitGetAnIdleWorker();
        std::pair<Worker *, uint32_t> TryGetAnIdleWorker();
        Worker *GetWorkerById(uint32_t id);
        void StartWorker(uint32_t workerId, void *arg);
        uint32_t WorkerCapacity() const { return _workerCapacity; }
        uint32_t WorkerNum() const { return _workerNum; }
        Worker *PoolWorker(uint32_t workerId) const { return workerId < _workerNum ? _vecWorker[workerId] : nullptr; }
        bool IsStarted() const { return _started; }
        void Stop();
        void WaitAllJobsFinished();

    private:
        bool FindIdleWorkerUnsafe(uint32_t *workerId);

    private:
        uint32_t _workerNum = 0;
        uint32_t _workerCapacity = 0;
        //Owner, need deconstruction
        std::vector<Worker *> _vecWorker;
        std::condition_variable _poolCondition = {};
        std::mutex _poolMutex = {};
        std::vector<std::unique_ptr<std::condition_variable>> _vecWorkerCondition;
        std::vector<Status> _vecWorkerStatus;
        //Not Owner, don't need deconstruction
        std::vector<void *> _vecWorkerArgument;
        bool _started = false;
    };
} // namespace sunflower

#endif //THREAD_POOL_H
