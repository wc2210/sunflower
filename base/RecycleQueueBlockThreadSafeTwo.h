#ifndef RECYCLEQUEUE_BLOCK_THREADSAFE_TWO_H
#define RECYCLEQUEUE_BLOCK_THREADSAFE_TWO_H

#include <stdint.h>
#include <string.h>
#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <memory>

#include "Common.h"

namespace sunflower {

template <typename stType>
class RecycleQueueBlockThreadSafeTwo
{
public:
  explicit RecycleQueueBlockThreadSafeTwo(uint32_t power = 10) : used_(0)
	{
    if (power > 32){
      power = 32; 
    } 
		capacity_ = 1 << power;
		mask_ = capacity_ - 1;
		queue_.resize(capacity_);
	}

  RecycleQueueBlockThreadSafeTwo(const RecycleQueueBlockThreadSafeTwo& other) =delete;
  RecycleQueueBlockThreadSafeTwo(RecycleQueueBlockThreadSafeTwo&& other) =delete;
  RecycleQueueBlockThreadSafeTwo& operator= (const RecycleQueueBlockThreadSafeTwo& other) =delete;
  RecycleQueueBlockThreadSafeTwo& operator= (RecycleQueueBlockThreadSafeTwo&& other) =delete;

  ~RecycleQueueBlockThreadSafeTwo()
  {
  }

  void clear()
  {		
    std::shared_ptr<stType> node = std::make_shared<stType>();
    while(tryPop(node));
  }

  size_t size() const { 
    return used_.load(); 
  }

  bool tryPush(const stType& node){ return tryPushTail(node);}
  bool tryPush(stType&& node){ return tryPushTail(std::move(node));}
  void waitPush(const stType& node){ waitPushTail(node);}
  void waitPush(stType&& node){ waitPushTail(std::move(node));} 
  bool tryPop(stType& node){ return tryPopHead(node);}
  stType waitPop(){ 
    stType value{};
    waitPopHead(value);
    return value;
  }
 
  bool full() const { 
    return used_.load() == capacity_;
  }
  bool empty() const { 
    return used_.load() == 0;
  }
  
private:
  template <typename T>
  void waitPushTail(T&& node)
  {
    {
      std::unique_lock<std::mutex> lck(mutexRead);
      cvFull.wait(lck, [this]{ return !full();});
 
      queue_[writePos_] = std::forward<T>(node);
      writePos_ = POS_MOD_BASE(writePos_);
    }
    used_++;
    cvEmpty.notify_one();
  }

  template <typename T>
  bool tryPushTail(T&& node)
  {
    {
      std::lock_guard<std::mutex> lck(mutexRead);

      if (full()) {   //full
        return false;
      }
   
      queue_[writePos_] = std::forward<T>(node);
      writePos_ = POS_MOD_BASE(writePos_); 
    }
    used_++;
    cvEmpty.notify_one();
    return true;
  }

  bool tryPopHead(stType& node)
  {
    {
      std::lock_guard<std::mutex> lck(mutexWrite);
      if (empty()){       //empty
        return false;
      }

      node = std::move(queue_[readPos_]);
      readPos_ = POS_MOD_BASE(readPos_ + 1);
    }
    used_--;
    cvFull.notify_one();
    return true;
  }
  
  void waitPopHead(stType& node)
  {
    {
      std::unique_lock<std::mutex> lck(mutexWrite);
      cvEmpty.wait(lck, [this]{ return !empty();});
    
      node = std::move(queue_[readPos_]);
      readPos_ = POS_MOD_BASE(readPos_ + 1);
    }
    used_--;
    cvFull.notify_one();
  }

private:
  std::vector<stType>     queue_;
  size_t                  readPos_    = 0;
  size_t                  writePos_   = 0;
  uint32_t                capacity_   = 0;
  uint32_t                mask_       = 0;
  std::atomic<uint32_t>   used_;

  std::mutex              mutexRead   = {};
  std::mutex              mutexWrite  = {};
  std::condition_variable cvEmpty     = {};
  std::condition_variable cvFull      = {};
  
};
}//namespace sunflower
#endif // RECYCLEQUEUE_BLOCK_THREADSAFE_H





