#include <iostream>
#include <zthread/zthread.h>
//#include <zthread/thread.h>

class Func1 : public zthread::Runnable {
  public:
    void run() {
      std::cout << "Hello,world from Func1" << std::endl;
    }
};

class Func2 : public zthread::Runnable {
  public:
    void run() {
      std::cout << "Hello,world from Func2" << std::endl;
    }
};

int main() {
  zthread::Thread t1(new Func1);
  t1.Wait();

  zthread::Thread t2(new Func2);
  t2.Wait();

  zthread::BlockingQueue<int, zthread::FastMutex> block_queue;
  block_queue.Add(100);
  block_queue.Add(200);
  std::cout << "block_queue.Size == " << block_queue.Size() << std::endl;

  return 0;
}
