#pragma once

#include "scheduler.hh"
#include "delayqueue.hh"
#include "threadpool.hh"
#include "log.hh"

#include <cstdlib>
#include <cassert>
#include <mutex>
#include <thread>
#include <condition_variable>


namespace sma
{

class ThreadScheduler final : public Scheduler
{
  using Lock = std::unique_lock<std::mutex>;


public:
  ~ThreadScheduler()
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
  ThreadScheduler(std::size_t nr_threads)
    : threadpool(std::move(Threadpool<>(nr_threads)))
  {
    LOG(DEBUG);
  }

  std::mutex mutex;
  Threadpool<> threadpool;
  DelayQueue<std::function<void()>> tasks;

public:
  template <std::size_t NrThreads>
  class Factory final : public Scheduler::Factory
  {
  public:
    std::unique_ptr<Scheduler> new_scheduler() const override
    {
      return std::unique_ptr<Scheduler>{new ThreadScheduler(NrThreads)};
    }
  };
};
}
