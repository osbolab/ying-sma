#include <sma/thread_scheduler.hpp>


namespace sma
{

thread_scheduler::thread_scheduler(std::size_t nthreads)
  : threads(nthreads)
{
  LOG(DEBUG);
}

thread_scheduler::~thread_scheduler()
{
  LOG(DEBUG);
  threads.join(true);
}

void thread_scheduler::schedule(std::function<void()> task, millis delay)
{
  assert(task);
  LOG(DEBUG) << "task in " << delay.count() << "ms";
  // Put the function pointer in a priority queue where it will pop after
  // delay
  tasks.push(std::move(task), delay);
  // Starts a thread waiting on the task queue until something
  // becomes available. There will be a "pop and run" task for every function
  // that gets put in the queue and those will be divided among the threads.
  threadpool.push([this]() { tasks.pop()(); });
}
}
