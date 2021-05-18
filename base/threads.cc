#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "threads.h"

bool threads_new_rt(pthread_t* thread, void* (*start_routine)(void*), void* arg)
{
  return threads_new_rt_prio(thread, start_routine, arg, -1);
}

bool threads_new_rt_prio(pthread_t* thread, void* (*start_routine)(void*), void* arg, int prio_offset)
{
  return threads_new_rt_cpu(thread, start_routine, arg, -1, prio_offset);
}

bool threads_new_rt_mask(pthread_t* thread, void* (*start_routine)(void*), void* arg, int mask, int prio_offset)
{
  return threads_new_rt_cpu(thread,
                            start_routine,
                            arg,
                            mask * 1000,
                            prio_offset); // we multiply mask by 100 to distinguish it from a single cpu core id
}

bool threads_new_rt_cpu(pthread_t* thread, void* (*start_routine)(void*), void* arg, int cpu, int prio_offset)
{
  bool ret = false;

  pthread_attr_t     attr;
  struct sched_param param;
  cpu_set_t          cpuset;
  bool               attr_enable = false;

#ifdef PER_THREAD_PRIO
  if (prio_offset >= 0) {
    param.sched_priority = sched_get_priority_max(SCHED_FIFO) - prio_offset;
    pthread_attr_init(&attr);
    if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) {
      perror("pthread_attr_setinheritsched");
    }
    if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) {
      perror("pthread_attr_setschedpolicy");
    }
    if (pthread_attr_setschedparam(&attr, &param)) {
      perror("pthread_attr_setschedparam");
    }
    attr_enable = true;
  } else if (prio_offset == -1) {
    param.sched_priority = sched_get_priority_max(SCHED_FIFO) - DEFAULT_PRIORITY;
    pthread_attr_init(&attr);
    if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) {
      perror("pthread_attr_setinheritsched");
    }
    if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) {
      perror("pthread_attr_setschedpolicy");
    }
    if (pthread_attr_setschedparam(&attr, &param)) {
      perror("pthread_attr_setschedparam");
    }
    attr_enable = true;
  } else if (prio_offset == -2) {
#else
  // All threads have normal priority except prio_offset=0,1,2,3,4
  if (prio_offset >= 0 && prio_offset < 5) {
    param.sched_priority = 50 - prio_offset;
    if (pthread_attr_init(&attr)) {
      perror("pthread_attr_init");
    } else {
      attr_enable = true;
    }
    if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) {
      perror("pthread_attr_setinheritsched");
    }
    if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) {
      perror("pthread_attr_setschedpolicy");
    }
    if (pthread_attr_setschedparam(&attr, &param)) {
      perror("pthread_attr_setschedparam");
      fprintf(stderr, "Error not enough privileges to set Scheduling priority\n");
    }
  } else {
#endif
    param.sched_priority = 0;
    if (pthread_attr_init(&attr)) {
      perror("pthread_attr_init");
    } else {
      attr_enable = true;
    }
    if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) {
      perror("pthread_attr_setinheritsched");
    }
    if (pthread_attr_setschedpolicy(&attr, SCHED_OTHER)) {
      perror("pthread_attr_setschedpolicy");
    }
    if (pthread_attr_setschedparam(&attr, &param)) {
      perror("pthread_attr_setschedparam");
      fprintf(stderr, "Error not enough privileges to set Scheduling priority\n");
    }
  }
  if (cpu > 0) {
    if (cpu >= 1000) {
      uint32_t mask;
      mask = cpu / 1000;

      CPU_ZERO(&cpuset);
      for (uint32_t i = 0; i < 8; i++) {
        if (((mask >> i) & 0x01U) == 1U) {
          CPU_SET((size_t)i, &cpuset);
          break;
        }
      }
    } else {
      CPU_ZERO(&cpuset);
      CPU_SET((size_t)cpu, &cpuset);
    }

    if (pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset)) {
      perror("pthread_attr_setaffinity_np");
    }
  }

  int err = pthread_create(thread, attr_enable ? &attr : NULL, start_routine, arg);
  if (err) {
    if (EPERM == err) {
      // Join failed thread for avoiding memory leak from previous trial
      pthread_join(*thread, NULL);

      perror("Warning: Failed to create thread with real-time priority. Creating it with normal priority");
      err = pthread_create(thread, NULL, start_routine, arg);
      if (err) {
        perror("pthread_create");
      } else {
        ret = true;
      }
    } else {
      perror("pthread_create");
    }
  } else {
    ret = true;
  }
  if (attr_enable) {
    pthread_attr_destroy(&attr);
  }
  return ret;
}

void threads_print_self()
{
  pthread_t          thread;
  cpu_set_t          cpuset;
  struct sched_param param;
  int                policy;
  const char*        p;
  int                s, j;

  thread = pthread_self();

  s = pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
  if (s != 0) {
    printf("error pthread_getaffinity_np: %s\n", strerror(s));
  }

  printf("Set returned by pthread_getaffinity_np() contained:\n");
  for (j = 0; j < CPU_SETSIZE; j++) {
    if (CPU_ISSET(j, &cpuset)) {
      printf("    CPU %d\n", j);
    }
  }

  s = pthread_getschedparam(thread, &policy, &param);
  if (s != 0) {
    printf("error pthread_getaffinity_np: %s\n", strerror(s));
  }

  switch (policy) {
    case SCHED_FIFO:
      p = "SCHED_FIFO";
      break;
    case SCHED_RR:
      p = "SCHED_RR";
      break;
    default:
      p = "Other";
      break;
  }

  printf("Sched policy is %s. Priority is %d\n", p, param.sched_priority);
}
