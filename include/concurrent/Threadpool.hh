#pragma once

#include <cstdlib>
#include <thread>
#include <future>
#include <functional>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>


namespace sma
{

namespace detail
{

template<typename T>
using Container_queue__ = std::queue<T>;

}

template<template<typename> class Container = detail::Container_queue__>
class Threadpool
{
  using Lock = std::unique_lock<std::mutex>;

public:
  Threadpool(std::size_t nr_threads)
  {
    for (std::size_t i = 0; i < nr_threads; ++i)
      threads.emplace_back([this] {
      for (;;)
      {
        std::function<void()> task;

        {
          Lock lock(mutex);
          available.wait(lock, [this] { return stop || !tasks.empty(); });
          if (stop && tasks.empty()) return;
          task = std::move(tasks.front());
          tasks.pop();
        }

        task();
      }
    });
  }

  ~Threadpool()
  {
    {
      Lock lock(mutex);
      stop = true;
    }
    available.notify_all();
    join();
  }

  template<typename F, typename... Args>
  auto push(F&& f, Args&& ... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> result = task->get_future();

    {
      Lock lock(mutex);
      if (stop) throw std::runtime_error("Threadpool is shut down");
      tasks.emplace([task]() { (*task)(); });
    }

    available.notify_one();
    return result;
  }

  void shutdown()
  {
    Lock lock(mutex);
    stop = true;
  }

  void join(bool shutdown = false)
  {
    if (shutdown) this->shutdown();
    for (std::thread& thread : threads)
      if (thread.joinable()) thread.join();
  }

  std::size_t nr_threads() const
  {
    return threads.size();
  }

  std::size_t size() const
  {
    return tasks.size();
  }

private:
  std::vector<std::thread> threads;
  Container<std::function<void()>> tasks;

  std::mutex mutex;
  std::condition_variable available;
  volatile bool stop { false };
};

}