#ifndef THREADS_H
#define THREADS_H

#include "Noncopyable.h"
#include <iostream>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

#include <string>

namespace sunflower
{
    class Thread : public Noncopyable
    {
    public:
        Thread() {}
        explicit Thread(const std::string &name) : _name(name) {}

        virtual ~Thread(){};

        bool Start()
        {
            bool ret = false;
            int err = pthread_create(&_thread, nullptr, ThreadFunction, this);
            if (err)
            {
                std::cerr << "pthread_create";
            }
            else
            {
                ret = true;
                _started = true;
            }
            return ret;
        };

        void SetName(const std::string &name)
        {
            _name = name;
            if (_started)
                pthread_setname_np(pthread_self(), _name.c_str());
        }

        void WaitThreadFinish()
        {
            if (_started)
                pthread_join(_thread, NULL);
        }

        void ThreadCancel()
        {
            if (_started)
                pthread_cancel(_thread);
        }

        static std::string GetName()
        {
            const uint32_t MAX_LEN = 16;
            char name[MAX_LEN] = {};
            const pthread_t tid = pthread_self();
            if (pthread_getname_np(tid, name, MAX_LEN))
            {
                std::cerr << "Could not get pthread name";
            }
            return std::string(name);
        }

    protected:
        virtual void RunThread() = 0;

    private:
        static void *ThreadFunction(void *_this)
        {
            pthread_setname_np(pthread_self(), ((Thread *)_this)->_name.c_str());
            ((Thread *)_this)->RunThread();
            return NULL;
        }

        pthread_t _thread = 0;
        bool _started = false;
        std::string _name;
    };
} //namespace sunflower

#endif // THREADS_H
