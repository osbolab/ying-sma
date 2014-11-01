#pragma once

#include "scheduler.hh"
#include "delayqueue.hh"
#include "threadpool.hh"

#include <cstdlib>
#include <mutex>
#include <thread>
#include <condition_variable>


namespace sma
{

class ThreadScheduler final : public Scheduler
{
  using Lock = std::unique_lock<std::mutex>;


protected:
  // When the front-end scheduler creates a task it stores it by its ID;
  // this function should schedule that ID and call Scheduler::run when it's
  // ready.
  void schedule(std::function<void()> task, millis delay)
  {
    // Put the function pointer in a priority queue where it will pop after
    // delay
    tasks.push(task, delay);
    // Add a task that sets a thread to waiting on the queue until something
    // becomes available. There will be a "pop and run" task for every function
    // that gets put in the queue.
    threadpool.push([this]() { tasks.pop()(); });
  }


private:
  ThreadScheduler(std::size_t nr_threads)
    : threadpool(std::move(Threadpool<>(nr_threads)))
  {
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
