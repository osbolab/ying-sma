#pragma once

#include <sma/scheduler.hpp>
#include <sma/delay_queue.hpp>
#include <sma/threadpool.hpp>
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
  ~thread_scheduler();

protected:
  // The front-end scheduler packages the task function and its arguments for
  // delayed execution; the function scheduled here just invokes the wrapper.
  void schedule(std::function<void()> task, millis delay) override;

private:
  thread_scheduler(std::size_t nthreads);

  std::mutex mutex;
  threadpool threads;
  delay_queue<std::function<void()>> tasks;

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
