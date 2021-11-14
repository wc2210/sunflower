#include "ThreadPool.h"
#include <unistd.h>
#include <memory>
namespace sunflower
{
    void ThreadPool::Worker::Init(uint32_t workerId, ThreadPool *parent)
    {
        _workerId = workerId;
        _parent = parent;
        _parentMutex = &parent->_poolMutex;
        _parentCondition = &parent->_poolCondition;
        _condition = parent->_vecWorkerCondition[workerId].get();
        _status = &parent->_vecWorkerStatus[workerId];
        _arg = &parent->_vecWorkerArgument[workerId];
        std::string name = "ThreadPool_Worker" + std::to_string(_workerId);
        SetName(name);
        Start();
    }

    void ThreadPool::Worker::RunThread()
    {
        while (_parent->IsStarted())
        {
            //Wait for worker's condition variable
            {
                std::unique_lock<std::mutex> lck(*_parentMutex);
                _condition->wait(lck, [this]
                                 { return !(_parent->_started && *_status != Status::START); });
                *_status = Status::WORKING;
            }

            //Worker callback function
            RunWorker(*_arg);

            //Set worker status to IDLE, notify thread pool
            Finished();
        }
    }

    void ThreadPool::Worker::Finished()
    {
        {
            std::lock_guard<std::mutex> lck(*_parentMutex);
            *_status = Status::IDLE;
            *_arg = nullptr;
        }
        _condition->notify_one();
        _parentCondition->notify_one();
    }

    ThreadPool::ThreadPool(int workerCapacity)
    {
        _workerCapacity = (int)sysconf(_SC_NPROCESSORS_CONF);
        if (workerCapacity > 0 && workerCapacity < (int)_workerCapacity)
        {
            _workerCapacity = (uint32_t)workerCapacity;
        }

        _vecWorker.resize(_workerCapacity);
        _vecWorkerCondition.resize(_workerCapacity);
        _vecWorkerStatus.resize(_workerCapacity);
        _vecWorkerArgument.resize(_workerCapacity);

        for (uint32_t i = 0; i < _workerCapacity; i++)
        {
            _vecWorker[i] = nullptr;
            _vecWorkerArgument[i] = nullptr;
            _vecWorkerCondition[i] = std::make_unique<std::condition_variable>();
            _vecWorkerStatus[i] = Status::IDLE;
        }
        _started = true;
    }

    ThreadPool::~ThreadPool()
    {
        Stop();
    }

    int ThreadPool::RegisterWorker(Worker *worker)
    {
        uint32_t id = 0;
        {
            std::lock_guard<std::mutex> lock(_poolMutex);
            if (_workerNum == _workerCapacity)
            {
                std::cerr << "Worker num bigger than capacity!";
                return -1;
            }

            id = _workerNum++;
            _vecWorker[id] = worker;
            worker->Init(id, this);
        }
        _poolCondition.notify_one();
        return id;
    }

    bool ThreadPool::DoTask(void *arg, bool block)
    {
        bool ret = true;
        if (block)
        {
            auto worker = WaitGetAnIdleWorker();
            StartWorker(worker.second, arg);
        }
        else
        {
            auto worker = TryGetAnIdleWorker();
            if (nullptr == worker.first)
            {
                ret = false;
            }
            else
            {
                StartWorker(worker.second, arg);
            }
        }
        return ret;
    }

    bool ThreadPool::FindIdleWorkerUnsafe(uint32_t *workerId)
    {
        bool ret = false;
        for (uint32_t i = 0; i < _workerNum; i++)
        {
            if (_vecWorkerStatus[i] == Status::IDLE)
            {
                *workerId = i;
                ret = true;
                break;
            }
        }
        return ret;
    }

    std::pair<ThreadPool::Worker *, uint32_t> ThreadPool::WaitGetAnIdleWorker()
    {
        std::unique_lock<std::mutex> lck(_poolMutex);

        uint32_t workerId = 0;

        _poolCondition.wait(lck, [&]
                            { return !(_started && !FindIdleWorkerUnsafe(&workerId)); });
        //Go to the next state,prevent other worker wake up
        _vecWorkerStatus[workerId] = Status::READY;
        return std::make_pair(_vecWorker[workerId], workerId);
    }

    void ThreadPool::WaitAllJobsFinished()
    {
        std::unique_lock<std::mutex> lck(_poolMutex);

        for (uint32_t i = 0; i < _workerNum; i++)
        {
            _poolCondition.wait(lck, [&]
                                { return !(_started && _vecWorkerStatus[i] != Status::IDLE); });
        }
    }

    std::pair<ThreadPool::Worker *, uint32_t> ThreadPool::TryGetAnIdleWorker()
    {
        std::unique_lock<std::mutex> lck(_poolMutex);

        uint32_t workerId = 0;
        Worker *ret = nullptr;
        if (_started && FindIdleWorkerUnsafe(&workerId))
        {
            ret = _vecWorker[workerId];
            //Go to the next state,prevent other worker wake up
            _vecWorkerStatus[workerId] = Status::READY;
        }

        return std::make_pair(ret, workerId);
    }

    ThreadPool::Worker *ThreadPool::GetWorkerById(uint32_t id)
    {
        if (id < _workerNum)
        {
            return _vecWorker[id];
        }
        return nullptr;
    }

    void ThreadPool::StartWorker(uint32_t workerId, void *arg)
    {
        std::lock_guard<std::mutex> lck(_poolMutex);
        if (_started && workerId < _workerNum && _vecWorkerStatus[workerId] == Status::READY)
        {
            _vecWorkerArgument[workerId] = arg;
            _vecWorkerStatus[workerId] = Status::START;
            _vecWorkerCondition[workerId]->notify_one();
        }
    }

    void ThreadPool::Stop()
    {
        if (_started)
        {
            _started = false;

            //Wakeup all worker and it will finish
            for (uint32_t i = 0; i < _workerNum; i++)
            {
                _vecWorkerCondition[i]->notify_one();
                _vecWorker[i]->WaitThreadFinish();
                delete _vecWorker[i];
                _vecWorker[i] = nullptr;
            }
            _workerNum = 0;
        }
    }
} // namespace sunflower
