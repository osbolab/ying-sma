#pragma once

#include "scheduler.hh"
#include "delayqueue.hh"
#include "threadpool.hh"
#include "log.hh"

#include <mutex>
#include <thread>
#include <cstddef>
#include <condition_variable>


namespace sma
{

class ThreadScheduler final : public Scheduler
{
  using Lock = std::unique_lock<std::mutex>;

  using tasks_type = DelayQueue<std::function<void()>>;

public:
  ThreadScheduler(std::size_t nr_core_threads)
    : threadpool(std::move(Threadpool(nr_core_threads)))
    , tasks(std::make_unique<tasks_type>())
  {
  }

  ThreadScheduler(ThreadScheduler&& o) = default;
  ThreadScheduler& operator=(ThreadScheduler&& o) = default;

  ~ThreadScheduler()
  {
    threadpool.shutdown();
    threadpool.join();
  }

protected:
  void do_async(std::function<void()> task) override;
  // The front-end scheduler packages the task function and its arguments for
  // delayed execution; the function scheduled here just invokes the wrapper.
  void schedule(std::function<void()> task, millis delay_ms) override;

private:
  std::mutex mutex;
  Threadpool threadpool;
  // This gets accessed by threads in the pool so we'll need to pin a reference
  // for them in case we get moved.
  std::unique_ptr<tasks_type> tasks;

public:
  template <std::size_t NrThreads>
  class Factory final : public Scheduler::Factory
  {
  public:
    std::unique_ptr<Scheduler> new_scheduler() const override
    {
      return std::make_unique<ThreadScheduler>(NrThreads);
    }
  };
};
}
