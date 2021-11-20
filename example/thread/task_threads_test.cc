#include "base/TaskThreadPool.h"
#include <iostream>
#include <mutex>
#include <map>
#include <thread>

int test_task_thread_pool()
{
    std::mutex mutex;
    std::map<std::thread::id, int> count_worker;
    uint32_t total_runs = 100000000;

    static int a = 0;
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
    return 0;
}
