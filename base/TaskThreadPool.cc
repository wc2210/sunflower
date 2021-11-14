#include "TaskThreadPool.h"
#include <unistd.h>
namespace sunflower
{
    TaskThreadPool::Worker::Worker(TaskThreadPool *parent, uint32_t workerId)
        : _parent(parent), _workerId(workerId), Thread()
    {
        std::string name = "ThreadPool_Worker" + std::to_string(_workerId);
        SetName(name);
        Start();
    }

    bool TaskThreadPool::Worker::WaitTask(task_t *task)
    {
        if (not _parent->_started)
        {
            return false;
        }

        if (task)
        {
            *task = _parent->_pendingTask.waitPop();
        }

        return true;
    }

    void TaskThreadPool::Worker::RunThread()
    {
        // main loop
        task_t task;
        while (WaitTask(&task))
        {
            task();
        }
        _started = false;
    }
    void TaskThreadPool::Worker::Stop()
    {
        WaitThreadFinish();
    }

    TaskThreadPool::TaskThreadPool(uint32_t workerNum, uint32_t queuePow)
        : _pendingTask(queuePow)
    {
        _vecWorker.resize(workerNum);
        Start();
    }

    TaskThreadPool::~TaskThreadPool()
    {
        Stop();
    }

    void TaskThreadPool::SetWorkerNum(uint32_t num)
    {
        std::lock_guard<std::mutex> lck(_poolMutex);
        if (_vecWorker.size() >= num)
        {
            return;
        }
        size_t old = _vecWorker.size();
        _vecWorker.resize(num);
        if (_started)
        {
            for (size_t i = old; i < num; i++)
            {
                _vecWorker[i].reset(new Worker(this, i));
            }
        }
    }
    void TaskThreadPool::Start()
    {
        std::lock_guard<std::mutex> lck(_poolMutex);
        if (_started)
        {
            std::cout << "ThreadPool is already started" << std::endl;
            return;
        }

        _started = true;
        for (size_t i = 0; i < _vecWorker.size(); i++)
        {
            _vecWorker[i].reset(new Worker(this, i));
        }
    }

    void TaskThreadPool::Stop()
    {
        std::lock_guard<std::mutex> lck(_poolMutex);
        if (_started)
        {
            _started = false;
            for (auto &e : _vecWorker)
            {
                e->Stop();
            }
        }
    }
} // namespace sunflower
