#include <iostream>
#include <zthread/zthread.h>
//#include <zthread/thread.h>

class Func1 : public zthread::Runnable {
  public:
    void run() {
      std::cout << "Thread::Func Hello,world from Func1" << std::endl;
    }
};

class Func2 : public zthread::Runnable {
  public:
    void run() {
      std::cout << "Thread::Func Hello,world from Func2" << std::endl;
    }
};

class DemoFunc : public zthread::Runnable {
  public:
    void run() {
      std::cout << "Concurrent::Func DemoFunc" << std::endl;
    }
};

class SimpleClass {

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

  zthread::BoundedQueue<int, zthread::FastMutex> bounded_queue(5);
  bounded_queue.Add(100);
  bounded_queue.Add(200);
  std::cout << "bounded_queue.Size == " << bounded_queue.Size() << std::endl;

  // TODO: why must block here? issue? by design?
  //std::cout << "bounded_queue.Empty = " << bounded_queue.Empty() << std::endl;
  //
  
  std::cout << "testing ClassLockable.." << std::endl;
  zthread::ClassLockable<SimpleClass, zthread::FastMutex> class_lock;

  zthread::ConcurrentExecutor executor;

   // Submit p Tasks
  for(size_t n = 0; n < 4; n++)
     executor.Execute(new DemoFunc);

  return 0;
}
