#pragma once

#include <sma/async>
#include <sma/collect/delay_queue.hpp>
#include <sma/concurrent/threadpool.hpp>
#include <sma/log>

#include <cstdlib>
#include <cassert>
#include <mutex>
#include <thread>
#include <condition_variable>


namespace sma
{

class threadpool_async final
{
public:
  static threadpool_async& instance()
  {
    static threadpool_async& instance;
    return instance;
  }

protected:
  // The front-end scheduler packages the task function and its arguments for
  // delayed execution; the function scheduled here just invokes the wrapper.
  virtual void schedule(std::function<void()> task, millis delay) override;

private:
  using Lock = std::unique_lock<std::mutex>;

  threadpool_async();
  threadpool_async(threadpool_async const&);
  void operator=(threadpool_async const&);

  std::mutex mutex;
  threadpool threads;
  delay_queue<std::function<void()>> tasks;
};
}
