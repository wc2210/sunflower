#ifndef RECYCLEQUEUE_BLOCK_THREADSAFE_H
#define RECYCLEQUEUE_BLOCK_THREADSAFE_H

#include <stdint.h>
#include <string.h>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "Common.h"

namespace sunflower
{

    template <typename stType>
    class RecycleQueueBlockThreadSafe
    {
    public:
        explicit RecycleQueueBlockThreadSafe(uint32_t power = 10)
        {
            if (power > 32)
            {
                power = 32;
            }
            capacity_ = 1 << power;
            mask_ = capacity_ - 1;
            queue_.resize(capacity_);
        }

        RecycleQueueBlockThreadSafe(const RecycleQueueBlockThreadSafe &other) = delete;
        RecycleQueueBlockThreadSafe(RecycleQueueBlockThreadSafe &&other) = delete;
        RecycleQueueBlockThreadSafe &operator=(const RecycleQueueBlockThreadSafe &other) = delete;
        RecycleQueueBlockThreadSafe &operator=(RecycleQueueBlockThreadSafe &&other) = delete;

        ~RecycleQueueBlockThreadSafe()
        {
        }

        void clear()
        {
            stType node;
            while (tryPop(&node))
                ;
        }

        size_t size() const
        {
            std::lock_guard<std::mutex> lck(mutex);
            return POS_MOD_BASE(writePos_ - readPos_);
        }

        bool full() const
        {
            std::lock_guard<std::mutex> lck(mutex);
            return fullNosafe();
        }
        bool empty() const
        {
            std::lock_guard<std::mutex> lck(mutex);
            return emptyNosafe();
        }

        bool tryPush(const stType &node) { return push_(node, false); }
        bool tryPush(stType &&node) { return push_(std::move(node), false); }
        void waitPush(const stType &node) { push_(node, true); }
        void waitPush(stType &&node) { push_(std::move(node), true); }
        bool tryPop(stType &node) { return pop_(node, false); }
        stType waitPop()
        {
            stType value{};
            pop_(value, true);
            return value;
        }

    private:
        bool fullNosafe() const
        {
            return POS_MOD_BASE(writePos_ + 1) == readPos_;
        }
        bool emptyNosafe() const
        {
            return writePos_ == readPos_;
        }

        template <typename T>
        bool push_(T &&node, bool block) //万能引用
        {
            {
                std::unique_lock<std::mutex> lck(mutex);

                if (!block)
                {
                    if (fullNosafe())
                    { //full
                        return false;
                    }
                }
                else
                {
                    cv_full.wait(lck, [this]
                                 { return !(POS_MOD_BASE(writePos_ + 1) == readPos_); });
                }

                queue_[writePos_] = std::forward<T>(node);
                writePos_ = POS_MOD_BASE(writePos_ + 1);
            }
            cv_empty.notify_one();
            return true;
        }

        bool pop_(stType &node, bool block)
        {
            {
                std::unique_lock<std::mutex> lck(mutex);

                if (!block)
                {
                    if (emptyNosafe())
                    { //empty
                        return false;
                    }
                }
                else
                {
                    cv_empty.wait(lck, [this]
                                  { return !(writePos_ == readPos_); });
                }
                node = std::move(queue_[readPos_]);
                readPos_ = POS_MOD_BASE(readPos_ + 1);
            }
            cv_full.notify_one();
            return true;
        }

    private:
        std::vector<stType> queue_;
        size_t readPos_ = 0;
        size_t writePos_ = 0;
        uint32_t capacity_ = 0;
        uint32_t mask_ = 0;

        mutable std::mutex mutex = {};
        std::condition_variable cv_empty = {};
        std::condition_variable cv_full = {};
    };
} //namespace sunflower
#endif // RECYCLEQUEUE_BLOCK_THREADSAFE_H
