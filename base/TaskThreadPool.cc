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
            *task = std::move(_parent->_pendingTask.waitPop());
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
        : _workerNum(workerNum), _pendingTask(queuePow)
    {
        _vecWorker.resize(_workerNum);
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
        for (size_t i = 0; i < _workerNum; i++)
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

            auto stop = []() {};
            for (uint32_t i = 0; i < _workerNum; i++)
            {
                WaitPushTask(stop);
            }

            for (auto &e : _vecWorker)
            {
                e->Stop();
            }
        }
    }

    TaskThreadPoolQueues::Worker::Worker(TaskThreadPoolQueues *parent, uint32_t workerId, uint32_t queuePow)
        : _parent(parent), _workerId(workerId), _pendingTask(queuePow), Thread()
    {
        std::string name = "ThreadPool_Worker" + std::to_string(_workerId);
        SetName(name);
        Start();
    }

    bool TaskThreadPoolQueues::Worker::WaitTask(task_t *task)
    {
        while (_pendingTask.size() == 0)
        {
            usleep(50);
        }

        if (not _parent->_started)
        {
            return false;
        }

        if (task)
        {
            *task = std::move(_pendingTask.top());
            _pendingTask.pop();
        }

        return true;
    }

    void TaskThreadPoolQueues::Worker::RunThread()
    {
        // main loop
        task_t task;
        while (WaitTask(&task))
        {
            task();
        }
        _started = false;
    }
    void TaskThreadPoolQueues::Worker::Stop()
    {
        WaitThreadFinish();
    }

    TaskThreadPoolQueues::TaskThreadPoolQueues(uint32_t workerNum, uint32_t queuePow)
        : _workerNum(workerNum), _queuePow(queuePow)
    {
        _vecWorker.resize(_workerNum);
        Start();
    }

    TaskThreadPoolQueues::~TaskThreadPoolQueues()
    {
        Stop();
    }

    void TaskThreadPoolQueues::SetWorkerNum(uint32_t num)
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
                _vecWorker[i].reset(new Worker(this, i, _queuePow));
            }
        }
    }
    void TaskThreadPoolQueues::Start()
    {
        std::lock_guard<std::mutex> lck(_poolMutex);
        if (_started)
        {
            std::cout << "ThreadPool is already started" << std::endl;
            return;
        }

        _started = true;
        for (size_t i = 0; i < _workerNum; i++)
        {
            _vecWorker[i].reset(new Worker(this, i, _queuePow));
        }
    }

    void TaskThreadPoolQueues::Stop()
    {
        std::lock_guard<std::mutex> lck(_poolMutex);
        if (_started)
        {
            _started = false;

            auto stop = []() {};
            for (uint32_t i = 0; i < _workerNum; i++)
            {
                while (not _vecWorker[i]->_pendingTask.tryPush(stop))
                {
                    usleep(50);
                }
            }

            for (auto &e : _vecWorker)
            {
                e->Stop();
            }
        }
    }

    bool TaskThreadPoolQueues::TryPushTask(task_t &&task)
    {
        std::lock_guard<std::mutex> lck(_poolMutex);
        for (uint32_t i = 1; i <= _workerNum; i++)
        {
            uint32_t pos = (_lastPos + i) % _workerNum;
            if (not _vecWorker[pos]->_pendingTask.full())
            {
                _vecWorker[pos]->_pendingTask.tryPush(task);
                _lastPos = pos;
                return true;
            }
        }
        return false;
    }

    uint32_t TaskThreadPoolQueues::GetTaskNum()
    {
        uint32_t num = 0;
        std::lock_guard<std::mutex> lck(_poolMutex);

        for (uint32_t i = 0; i < _workerNum; i++)
        {
            num += _vecWorker[i]->_pendingTask.size();
        }
        return num;
    }
} // namespace sunflower
