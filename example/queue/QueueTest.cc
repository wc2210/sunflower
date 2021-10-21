#include "base/RecycleQueue.h"
#include "base/RecycleQueueBlockThreadSafe.h"
#include "base/RecycleQueueBlockThreadSafeTwo.h"
#include <sys/time.h>
#include <thread>
#include <vector>
#include <iostream>
#include <unistd.h>

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
void QueueTryRead(T& queue, const char* thread_name, int cnt)
{
  int n = cnt;
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
  
  printf("%s read time interval:%luus\n", thread_name, timestamp[0].tv_sec * 1000000 + timestamp[0].tv_usec);
}

template <typename T>
void QueueWaitRead(T& queue, const char* thread_name, int cnt)
{
  int n = cnt;
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
  
  printf("%s read time interval:%luus\n", thread_name, timestamp[0].tv_sec * 1000000 + timestamp[0].tv_usec);
}


template <typename T>
void QueueTryWrite(T& queue, const char* thread_name, int cnt)
{
  int n = cnt;
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
  
  printf("%s write time interval:%luus\n", thread_name, timestamp[0].tv_sec * 1000000 + timestamp[0].tv_usec);
}

template <typename T>
void QueueWaitWrite(T& queue, const char* thread_name, int cnt)
{
  int n = cnt;
  struct timeval timestamp[3];

	gettimeofday(&timestamp[1], NULL);

	while (n > 0) {
  	queue.waitPush(n);
    n--;
  }

  gettimeofday(&timestamp[2], NULL);
  
  GetTimeInterval(timestamp);
  
  printf("%s write time interval:%luus\n", thread_name, timestamp[0].tv_sec * 1000000 + timestamp[0].tv_usec);
}

void test1()
{
  RecycleQueue<int> queue(20);
  std::vector<std::thread> vecThread;
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueue<int>>, std::ref(queue), "test1_tryread_1", 100000000));
  vecThread.push_back(std::thread(QueueTryWrite<RecycleQueue<int>>, std::ref(queue), "test1_trywrite_1", 100000000));
  for (auto& e : vecThread) {
		e.join();
	}
	std::cout << "RecycleQueue queue size: " << queue.size() << std::endl;

}

void test2()
{
  RecycleQueueBlockThreadSafe<int> queue(20);
  std::vector<std::thread> vecThread;
	vecThread.push_back(std::thread(QueueTryWrite<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test2_trywrite_1", 100000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test2_tryread_1", 100000000));
  for (auto& e : vecThread) {
		e.join();
	}
	std::cout << "RecycleQueueBlockThreadSafe queue size: " << queue.size() << std::endl;
}

void test3()
{
  RecycleQueueBlockThreadSafe<int> queue(20);
  std::vector<std::thread> vecThread;
	vecThread.push_back(std::thread(QueueTryWrite<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test3_trywrite_1", 100000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test3_tryread_1", 50000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test3_tryread_2", 50000000));
  for (auto& e : vecThread) {
		e.join();
	}
	std::cout << "RecycleQueueBlockThreadSafe queue size: " << queue.size() << std::endl;
}

void test4()
{
  RecycleQueueBlockThreadSafe<int> queue(20);
  std::vector<std::thread> vecThread;
	vecThread.push_back(std::thread(QueueTryWrite<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test4_trywrite_1", 100000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test4_tryread_1", 25000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test4_tryread_2", 25000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test4_tryread_3", 25000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test4_tryread_4", 25000000));
  for (auto& e : vecThread) {
		e.join();
	}
	std::cout << "RecycleQueueBlockThreadSafe queue size: " << queue.size() << std::endl;
}

void test5()
{
  RecycleQueueBlockThreadSafe<int> queue(20);
  std::vector<std::thread> vecThread;
	vecThread.push_back(std::thread(QueueWaitWrite<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test5_waitwrite_1", 100000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test5_waitread_1", 100000000));
  for (auto& e : vecThread) {
		e.join();
	}
	std::cout << "RecycleQueueBlockThreadSafe queue size: " << queue.size() << std::endl;
}

void test6()
{
  RecycleQueueBlockThreadSafe<int> queue(20);
  std::vector<std::thread> vecThread;
	vecThread.push_back(std::thread(QueueWaitWrite<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test6_waitwrite_1", 100000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test6_waitread_1", 50000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test6_waitread_2", 50000000));
  for (auto& e : vecThread) {
		e.join();
	}
	std::cout << "RecycleQueueBlockThreadSafe queue size: " << queue.size() << std::endl;
}

void test7()
{
  RecycleQueueBlockThreadSafe<int> queue(20);
  std::vector<std::thread> vecThread;
	vecThread.push_back(std::thread(QueueWaitWrite<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test7_waitwrite_1", 100000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test7_waitread_1", 25000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test7_waitread_2", 25000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test7_waitread_3", 25000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafe<int>>, std::ref(queue), "test7_waitread_4", 25000000));
  for (auto& e : vecThread) {
		e.join();
	}
	std::cout << "RecycleQueueBlockThreadSafe queue size: " << queue.size() << std::endl;
}

void test8()
{
  RecycleQueueBlockThreadSafeTwo<int> queue(20);
  std::vector<std::thread> vecThread;
	vecThread.push_back(std::thread(QueueTryWrite<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test8_trywrite_1", 100000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test8_tryread_1", 100000000));
  for (auto& e : vecThread) {
		e.join();
	}
	std::cout << "RecycleQueueBlockThreadSafeTwo queue size: " << queue.size() << std::endl;
}

void test9()
{
  RecycleQueueBlockThreadSafeTwo<int> queue(20);
  std::vector<std::thread> vecThread;
	vecThread.push_back(std::thread(QueueTryWrite<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test9_trywrite_1", 100000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test9_tryread_1", 50000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test9_tryread_2", 50000000));
  for (auto& e : vecThread) {
		e.join();
	}
	std::cout << "RecycleQueueBlockThreadSafeTwo queue size: " << queue.size() << std::endl;
}

void test10()
{
  RecycleQueueBlockThreadSafeTwo<int> queue(20);
  std::vector<std::thread> vecThread;
	vecThread.push_back(std::thread(QueueTryWrite<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test10_trywrite_1", 100000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test10_tryread_1", 25000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test10_tryread_2", 25000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test10_tryread_3", 25000000));
  vecThread.push_back(std::thread(QueueTryRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test10_tryread_4", 25000000));
  for (auto& e : vecThread) {
		e.join();
	}
	std::cout << "RecycleQueueBlockThreadSafeTwo queue size: " << queue.size() << std::endl;
}

void test11()
{
  RecycleQueueBlockThreadSafeTwo<int> queue(20);
  std::vector<std::thread> vecThread;
	vecThread.push_back(std::thread(QueueWaitWrite<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test11_waitwrite_1", 100000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test11_waitread_1", 100000000));
  for (auto& e : vecThread) {
		e.join();
	}
	std::cout << "RecycleQueueBlockThreadSafeTwo queue size: " << queue.size() << std::endl;
}

void test12()
{
  RecycleQueueBlockThreadSafeTwo<int> queue(20);
  std::vector<std::thread> vecThread;
	vecThread.push_back(std::thread(QueueWaitWrite<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test12_waitwrite_1", 100000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test12_waitread_1", 50000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test12_waitread_2", 50000000));
  for (auto& e : vecThread) {
		e.join();
	}
	std::cout << "RecycleQueueBlockThreadSafeTwo queue size: " << queue.size() << std::endl;
}

void test13()
{
  RecycleQueueBlockThreadSafeTwo<int> queue(20);
  std::vector<std::thread> vecThread;
	vecThread.push_back(std::thread(QueueWaitWrite<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test13_waitwrite_1", 100000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test13_waitread_1", 25000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test13_waitread_2", 25000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test13_waitread_3", 25000000));
  vecThread.push_back(std::thread(QueueWaitRead<RecycleQueueBlockThreadSafeTwo<int>>, std::ref(queue), "test13_waitread_4", 25000000));
  for (auto& e : vecThread) {
		e.join();
	}
	std::cout << "RecycleQueueBlockThreadSafeTwo queue size: " << queue.size() << std::endl;
}
int main(){
  RecycleQueueBlockThreadSafeTwo<int> queue3(20);
	RecycleQueueBlockThreadSafeTwo<int> queue3_2(20);

	//RecycleQueue, try read 1 write 1
  test1();  

	//RecycleQueueBlockThreadSafe, try read 1 write 1
  test2();
  
	//RecycleQueueBlockThreadSafe, try read 2 write 1
  test3();
  
	//RecycleQueueBlockThreadSafe, try read 4 write 1
  test4();

	//RecycleQueueBlockThreadSafe, wait read 1 write 1
  test5();
  
	//RecycleQueueBlockThreadSafe, wait read 2 write 1
  test6();
  
	//RecycleQueueBlockThreadSafe, wait read 4 write 1
  test7();

   //RecycleQueueBlockThreadSafeTwo, try read 1 write 1
  test8();
  
	//RecycleQueueBlockThreadSafeTwo, try read 2 write 1
  test9();
  
	//RecycleQueueBlockThreadSafeTwo, try read 4 write 1
  test10();

	//RecycleQueueBlockThreadSafeTwo, wait read 1 write 1
  test11();
  
	//RecycleQueueBlockThreadSafeTwo, wait read 2 write 1
  test12();
  
	//RecycleQueueBlockThreadSafeTwo, wait read 4 write 1
  test13();

  return 0;
}
