#pragma once

#include <sma/core/scheduler.hpp>
#include <sma/core/delay_queue.hpp>
#include <sma/core/threadpool.hpp>
#include <sma/log.hpp>

#include <cstdlib>
#include <cassert>
#include <mutex>
#include <thread>
#include <condition_variable>


namespace sma
{

class thread_scheduler final : public scheduler
{
  using Lock = std::unique_lock<std::mutex>;


public:
  ~thread_scheduler()
  {
    LOG(DEBUG);
    threadpool.join(true);
  }

protected:
  // The front-end scheduler packages the task function and its arguments for
  // delayed execution; the function scheduled here just invokes the wrapper.
  void schedule(std::function<void()> task, millis delay)
  {
    assert(task);
    LOG(DEBUG) << "task in " << delay.count() << "ms";
    // Put the function pointer in a priority queue where it will pop after
    // delay
    tasks.push(std::move(task), delay);
    // Starts a thread waiting on the task queue until something
    // becomes available. There will be a "pop and run" task for every function
    // that gets put in the queue and those will be divided among the threads.
    threadpool.push([this]() {
      LOG(DEBUG) << "<scheduler threadpool pop task>";
      tasks.pop()();
    });
  }


private:
  thread_scheduler(std::size_t nthreads)
    : threadpool(std::move(threadpool<>(nthreads)))
  {
    LOG(DEBUG);
  }

  std::mutex mutex;
  threadpool<> threadpool;
  delayqueue<std::function<void()>> tasks;

public:
  template <std::size_t NThreads>
  class factory final : public scheduler::factory
  {
  public:
    std::unique_ptr<scheduler> new_scheduler() const override
    {
      return std::unique_ptr<scheduler>{new thread_scheduler(NThreads)};
    }
  };
};
}
