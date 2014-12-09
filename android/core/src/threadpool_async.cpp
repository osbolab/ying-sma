#include <sma/async>
#include <sma/android/detail/threadpool_async.hpp>


namespace sma
{
namespace detail
{
  threadpool_async::threadpool_async(std::size_t nthreads)
    : threads(nthreads)
  {
    LOG(DEBUG);
  }

  threadpool_async::~threadpool_async()
  {
    LOG(DEBUG);
    threads.join();
  }

  void async_scheduler::schedule(std::function<void()> task, millis delay)
  {
    assert(task);
    LOG(DEBUG) << "task in " << delay.count() << "ms";
    // Put the function pointer in a priority queue where it will pop after
    // delay
    tasks.push(std::move(task), delay);
    // Starts a thread waiting on the task queue until something
    // becomes available. There will be a "pop and run" task for every function
    // that gets put in the queue and those will be divided among the threads.
    threads.push_back([this]() { tasks.pop()(); });
  }
}
}
