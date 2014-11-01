#pragma once

#include "scheduler.hh"
#include "delayqueue.hh"

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
  void schedule(detail::taskid_type taskid, millis delay) override;


private:
  ThreadScheduler(std::size_t nr_threads)
    : nr_threads(nr_threads)
  {
  }

  std::size_t nr_threads;
  std::mutex mutex;
  DelayQueue<detail::taskid_type> tasks;


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
