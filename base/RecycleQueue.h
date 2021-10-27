#ifndef RECYCLEQUEUE_H
#define RECYCLEQUEUE_H

#include <stdint.h>
#include <string.h>
#include <vector>
#include <memory>
#include <assert.h>

#include "Common.h"

namespace sunflower
{

    template <typename stType>
    class RecycleQueue
    {
    public:
        explicit RecycleQueue(uint32_t power = 10)
        {
            capacity_ = 1 << power;
            mask_ = capacity_ - 1;
            queue_.resize(capacity_);
        }

        RecycleQueue(const RecycleQueue &other) = delete;
        RecycleQueue(RecycleQueue &&other) = delete;
        RecycleQueue &operator=(const RecycleQueue &other) = delete;
        RecycleQueue &operator=(RecycleQueue &&other) = delete;

        ~RecycleQueue()
        {
        }

        void clear()
        {
            writePos_ = 0;
            readPos_ = 0;
        }

        bool full() const { return POS_MOD_BASE(writePos_ + 1) == readPos_; }
        bool empty() const { return writePos_ == readPos_; }

        size_t size() const { return POS_MOD_BASE(writePos_ - readPos_); }

        bool tryPush(const stType &node) { return push_(node); }
        bool tryPush(stType &&node) { return push_(std::move(node)); }

        stType &top()
        {
            assert(LIKELY(not empty()));
            return queue_[readPos_];
        }

        const stType &top() const
        {
            assert(LIKELY(not empty()));
            return queue_[readPos_];
        }

        void pop()
        {
            assert(LIKELY(not empty()));
            readPos_ = POS_MOD_BASE(readPos_ + 1);
        }

    private:
        template <typename T>
        bool push_(T &&node) //万能引用
        {
            if (full())
            {
                return false;
            }

            queue_[writePos_] = std::forward<T>(node);
            writePos_ = POS_MOD_BASE(writePos_ + 1);
            return true;
        }

    private:
        std::vector<stType> queue_;
        volatile size_t readPos_ = 0;
        volatile size_t writePos_ = 0;
        size_t capacity_ = 0;
        size_t mask_ = 0;
    };
} //namespace sunflower
#endif //RECYCLEQUEUE_H
