#ifndef BASE_THREADS_H
#define BASE_THREADS_H

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#ifdef PER_THREAD_PRIO
#define DEFAULT_PRIORITY 60 
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

bool threads_new_rt(pthread_t* thread, void* (*start_routine)(void*), void* arg);
bool threads_new_rt_prio(pthread_t* thread, void* (*start_routine)(void*), void* arg, int prio_offset);
bool threads_new_rt_cpu(pthread_t* thread, void* (*start_routine)(void*), void* arg, int cpu, int prio_offset);
bool threads_new_rt_mask(pthread_t* thread, void* (*start_routine)(void*), void* arg, int mask, int prio_offset);
void threads_print_self();

#ifdef __cplusplus
}

#include <string>

namespace sunflower {
	
class thread
{
public:
  thread(const std::string& name_) : _thread(0), name(name_) {}

  thread(const thread&) = delete;

  thread(thread&& other) noexcept
  {
    _thread       = other._thread;
    name          = std::move(other.name);
    other._thread = 0;
    other.name    = "";
  }

  virtual ~thread() = default;

  thread& operator=(const thread&) = delete;

  thread& operator=(thread&&) noexcept = delete;

  bool start(int prio = -1) { return threads_new_rt_prio(&_thread, thread_function_entry, this, prio); }

  bool start_cpu(int prio, int cpu) { return threads_new_rt_cpu(&_thread, thread_function_entry, this, cpu, prio); }

  bool start_cpu_mask(int prio, int mask)
  {
    return threads_new_rt_mask(&_thread, thread_function_entry, this, mask, prio);
  }

  void print_priority() { threads_print_self(); }

  void set_name(const std::string& name_)
  {
    name = name_;
    pthread_setname_np(pthread_self(), name.c_str());
  }

  void wait_thread_finish() { pthread_join(_thread, NULL); }

  void thread_cancel() { pthread_cancel(_thread); }

  static std::string get_name()
  {
    const uint32_t  MAX_LEN       = 16;
    char            name[MAX_LEN] = {};
    const pthread_t tid           = pthread_self();
    if (pthread_getname_np(tid, name, MAX_LEN)) {
      perror("Could not get pthread name");
    }
    return std::string(name);
  }

protected:
  virtual void run_thread() = 0;

private:
  static void* thread_function_entry(void* _this)
  {
    pthread_setname_np(pthread_self(), ((thread*)_this)->name.c_str());
    ((thread*)_this)->run_thread();
    return NULL;
  }

  pthread_t   _thread;
  std::string name;
};
}//namespace sunflower

#endif // __cplusplus
#endif // BASE_THREADS_H
