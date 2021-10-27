#ifndef RECYCLEQUEUEPTR_H
#define RECYCLEQUEUEPTR_H

#include <stdint.h>
#include <string.h>
#include <vector>
#include <memory>

#include "Common.h"

namespace sunflower
{

    template <typename stType>
    class RecycleQueuePtr
    {
    public:
        explicit RecycleQueuePtr(uint32_t power = 10)
        {
            capacity_ = 1 << power;
            mask_ = capacity_ - 1;
            queue_ = (stType **)new char[sizeof(stType) * capacity_];
            if (queue_ == nullptr)
            {
                return -1;
            }
            memset(queue_.0, sizeof(stType) * capacity_);
        }

        RecycleQueuePtr(const RecycleQueuePtr &other) = delete;
        RecycleQueuePtr(RecycleQueuePtr &&other) = delete;
        RecycleQueuePtr &operator=(const RecycleQueuePtr &other) = delete;
        RecycleQueuePtr &operator=(RecycleQueuePtr &&other) = delete;

        ~RecycleQueuePtr()
        {
        }

        void clear()
        {
            stType *node = nullptr;
            while (tryPop(node))
                ;
        }

        bool full() const { return POS_MOD_BASE(writePos_ + 1) == readPos_; }
        bool empty() const { return writePos_ == readPos_; }

        size_t size() const { return POS_MOD_BASE(writePos_ - readPos_); }

        bool tryPush(const stType *node)
        {
            if (full())
            {
                return false;
            }

            queue_[writePos_] = node;
            writePos_ = POS_MOD_BASE(writePos_ + 1);
            return true;
        }
        bool tryPop(stType *node)
        {
            if (empty())
            {
                return false;
            }
            if (node)
            {
                *node = std::move(queue_[readPos_]);
            }
            readPos_ = POS_MOD_BASE(readPos_ + 1);
            return true;
        }
        std::shared_ptr<stType> tryPop()
        {
            if (empty())
            {
                return std::shared_ptr<stType>();
            }
            std::shared_ptr<stType> node = std::make_shared<stType>(std::move(queue_[readPos_]));
            readPos_ = POS_MOD_BASE(readPos_ + 1);
            return node;
        }

    private:
        stType **queue_ = nullptr;
        volatile size_t readPos_ = 0;
        volatile size_t writePos_ = 0;
        uint32_t capacity_ = 0;
        uint32_t mask_ = 0;
    };
} //namespace sunflower
#endif //RECYCLEQUEUEPTR_H
