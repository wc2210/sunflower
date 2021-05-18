#include "base/RecycleQueue.h"
#include "base/RecycleQueueBlockThreadSafe.h"
#include "base/RecycleQueueBlockThreadSafeTwo.h"
#include <sys/time.h>
#include <thread>
#include <vector>
#include <iostream>

using namespace sunflower;

inline void GetTimeInterval(struct timeval* tdata)
{

  tdata[0].tv_sec  = tdata[2].tv_sec - tdata[1].tv_sec;
  tdata[0].tv_usec = tdata[2].tv_usec - tdata[1].tv_usec;
  if (tdata[0].tv_usec < 0) {
    tdata[0].tv_sec--;
    tdata[0].tv_usec += 1000000;
  }
}

template <typename T>
void QueueTryRead(T& queue, const char* thread_name)
{
  int n = 1000;
  int value;
  struct timeval timestamp[3];

  gettimeofday(&timestamp[1], NULL);

	while (n > 0) {
    if (queue.tryPop(&value)){
			//std::cout << "Pop:" << value << std::endl;
    	n--;
    }
  }

  gettimeofday(&timestamp[2], NULL);
  
  GetTimeInterval(timestamp);
  
  printf("%s read time interval:%luus\n", thread_name, timestamp[0].tv_usec);
}

template <typename T>
void QueueWaitRead(T& queue, const char* thread_name)
{
  int n = 1000;
  struct timeval timestamp[3];
  int value;
  gettimeofday(&timestamp[1], NULL);

	while (n > 0) {
    value = queue.waitPop();
		//std::cout << "Pop:" << value << std::endl;
    n--;
  }

  gettimeofday(&timestamp[2], NULL);
  
  GetTimeInterval(timestamp);
  
  printf("%s read time interval:%luus\n", thread_name, timestamp[0].tv_usec);
}


template <typename T>
void QueueTryWrite(T& queue, const char* thread_name)
{
  int n = 1000;
  struct timeval timestamp[3];

	gettimeofday(&timestamp[1], NULL);

	while (n > 0) {
  	if(queue.tryPush(n)){
			//std::cout << "Push:" << n << std::endl;
    	n--;
  	}
  }

  gettimeofday(&timestamp[2], NULL);
  
  GetTimeInterval(timestamp);
  
  printf("%s write time interval:%luus\n", thread_name, timestamp[0].tv_usec);
}

template <typename T>
void QueueWaitWrite(T& queue, const char* thread_name)
{
  int n = 1000;
  struct timeval timestamp[3];

	gettimeofday(&timestamp[1], NULL);

	while (n > 0) {
  	queue.waitPush(n);
    n--;
  }

  gettimeofday(&timestamp[2], NULL);
  
  GetTimeInterval(timestamp);
  
  printf("%s write time interval:%luus\n", thread_name, timestamp[0].tv_usec);
}

int main(){
  RecycleQueue<int> queue1(10);
  RecycleQueueBlockThreadSafe<int> queue2(10);
	RecycleQueueBlockThreadSafe<int> queue2_2(10);
  RecycleQueueBlockThreadSafeTwo<int> queue3(10);
	RecycleQueueBlockThreadSafeTwo<int> queue3_2(10);

  std::vector<std::thread> vecThread;  

	//queue1 try test 1read 1write
  vecThread.push_back(std::thread(QueueTryWrite<RecycleQueue<int>>, std::ref(queue1), "queue1_trywrite_1"));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueue<int>>, std::ref(queue1), "queue1_tryread_1"));
	
	//queue2 try test 2read 2write
	vecThread.push_back(std::thread(QueueTryWrite<RecycleQueueBlockThreadSafe<int>>, std::ref(queue2), "queue2_trywrite_1"));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue2), "queue2_tryread_1"));
	vecThread.push_back(std::thread(QueueTryWrite<RecycleQueueBlockThreadSafe<int>>, std::ref(queue2), "queue2_trywrite_2"));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue2), "queue2_tryread_2"));

	//queue2 wait test 2read 2write
	vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue2_2), "queue2_waitread_1"));
	vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue2_2), "queue2_waitread_2"));
	vecThread.push_back(std::thread(QueueWaitWrite<RecycleQueueBlockThreadSafe<int>>, std::ref(queue2_2), "queue2_waitwrite_1"));
	vecThread.push_back(std::thread(QueueWaitWrite<RecycleQueueBlockThreadSafe<int>>, std::ref(queue2_2), "queue2_waitwrite_2"));
  
	//queue3 try test 2read 2write
	vecThread.push_back(std::thread(QueueTryWrite<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue3), "queue3_trywrite_1"));
	vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue3), "queue3_tryread_1"));
	vecThread.push_back(std::thread(QueueTryWrite<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue3), "queue3_trywrite_2"));
	vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue3), "queue3_tryread_2"));

	//queue3 wait test 2read 2write
	vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue3_2), "queue3_waitread_1"));
	vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue3_2), "queue3_waitread_2"));
	vecThread.push_back(std::thread(QueueWaitWrite<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue3_2), "queue3_waitwrite_1"));
	vecThread.push_back(std::thread(QueueWaitWrite<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue3_2), "queue3_waitwrite_2"));

	for (auto& e : vecThread) {
		e.join();
	}

	std::cout << "queue1 size: " << queue1.size() << std::endl;
	std::cout << "queue2 size: " << queue2.size() << std::endl;
	std::cout << "queue2_2 size: " << queue2_2.size() << std::endl;
	std::cout << "queue3 size: " << queue3.size() << std::endl;
	std::cout << "queue3_2 size: " << queue3_2.size() << std::endl;

  return 0;
}
