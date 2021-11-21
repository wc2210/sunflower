#include "base/TaskThreadPool.h"
#include <sys/time.h>
#include <iostream>
#include <mutex>
#include <map>
#include <thread>

inline void GetTimeInterval(struct timeval *tdata)
{

    tdata[0].tv_sec = tdata[2].tv_sec - tdata[1].tv_sec;
    tdata[0].tv_usec = tdata[2].tv_usec - tdata[1].tv_usec;
    if (tdata[0].tv_usec < 0)
    {
        tdata[0].tv_sec--;
        tdata[0].tv_usec += 1000000;
    }
}

int test_task_thread_pool()
{
    std::mutex mutex;
    std::map<std::thread::id, int> count_worker;
    uint32_t total_runs = 1000000;
    struct timeval timestamp[3];
    gettimeofday(&timestamp[1], NULL);

    sunflower::TaskThreadPool task_pool(4, 20);
    auto task = [&count_worker, &mutex]()
    {
        std::lock_guard<std::mutex> lck(mutex);
        count_worker[std::this_thread::get_id()]++;
    };

    for (uint32_t i = 0; i < total_runs; i++)
    {
        task_pool.WaitPushTask(task);
    }

    while (task_pool.GetTaskNum() > 0)
    {
        usleep(100);
    }
    task_pool.Stop();

    gettimeofday(&timestamp[2], NULL);

    GetTimeInterval(timestamp);

    printf("task_thread_pool interval:%luus\n", timestamp[0].tv_sec * 1000000 + timestamp[0].tv_usec);

    uint32_t total_count = 0;
    for (auto &w : count_worker)
    {
        if (w.second < 10)
        {
            std::cout << "WARNING: the number of tasks " << w.second << " assigned to worker " << w.first << " is too low";
        }
        total_count += w.second;
        std::cout << "worker " << w.first << ": " << w.second << " runs\n";
    }
    if (total_count != total_runs)
    {
        printf("Number of task runs=%d does not match total=%d\n", total_count, total_runs);
        return -1;
    }

    std::cout << "outcome: Success\n";
    std::cout << "===================================================\n";
    return 0;
}

int test_task_thread_pool_queues()
{
    std::mutex mutex;
    std::map<std::thread::id, int> count_worker;
    uint32_t total_runs = 1000000;

    struct timeval timestamp[3];
    gettimeofday(&timestamp[1], NULL);

    sunflower::TaskThreadPoolQueues task_pool(4, 20);
    auto task = [&count_worker, &mutex]()
    {
        std::lock_guard<std::mutex> lck(mutex);
        count_worker[std::this_thread::get_id()]++;
    };

    for (uint32_t i = 0; i < total_runs; i++)
    {
        while (not task_pool.TryPushTask(task))
        {
            //std::cout << "task thread pool queue is full, id: " << i << std::endl;
            usleep(50);
        }
    }

    while (task_pool.GetTaskNum() > 0)
    {
        usleep(100);
    }
    task_pool.Stop();
    gettimeofday(&timestamp[2], NULL);

    GetTimeInterval(timestamp);

    printf("task_thread_pool_queues interval:%luus\n", timestamp[0].tv_sec * 1000000 + timestamp[0].tv_usec);

    uint32_t total_count = 0;
    for (auto &w : count_worker)
    {
        if (w.second < 10)
        {
            std::cout << "WARNING: the number of tasks " << w.second << " assigned to worker " << w.first << " is too low";
        }
        total_count += w.second;
        std::cout << "worker " << w.first << ": " << w.second << " runs\n";
    }
    if (total_count != total_runs)
    {
        printf("Number of task runs=%d does not match total=%d\n", total_count, total_runs);
        return -1;
    }

    std::cout << "outcome: Success\n";
    std::cout << "===================================================\n";
    return 0;
}
int main()
{
    test_task_thread_pool();
    test_task_thread_pool_queues();
    return 0;
}
