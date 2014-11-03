#pragma once

#include "log.hh"

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

  template <typename T>
  using Container_queue__ = std::queue<T>;
}

template <template <typename> class Container = detail::Container_queue__>
class Threadpool
{
  using Lock = std::unique_lock<std::mutex>;

public:
  Threadpool(std::size_t nr_threads)
  {
    LOG(DEBUG) << nr_threads << " threads";
    for (std::size_t i = 0; i < nr_threads; ++i) {
      LOG(DEBUG) << "constructing " << i << "th thread";
      threads.emplace_back([this] {
        LOG(DEBUG) << "child thread spawned";
        for (;;) {
          std::function<void()> task;

          {
            Lock lock(mutex);
            if (tasks.empty()) {
              LOG(DEBUG) << "waiting for task";
              available.wait(lock, [this] { return stop || !tasks.empty(); });
            }
            if (stop && tasks.empty())
              break;
            LOG(DEBUG) << "popping task";
            task = std::move(tasks.front());
            tasks.pop();
            LOG(DEBUG) << "popped task; running";
          }

          LOG(DEBUG) << "popped task; running";
          // task();
        }
        LOG(DEBUG) << "child thread dead";
      });
      LOG(DEBUG) << threads.size() << " threads constructed";
    }
  }

  ~Threadpool()
  {
    LOG(DEBUG) << "killing " << threads.size() << " threads";
    {
      Lock lock(mutex);
      stop = true;
    }
    available.notify_all();
    join();
  }

  template <typename F, typename... Args>
  auto push(F&& f, Args&&... args)
      -> std::future<typename std::result_of<F(Args...)>::type>
  {
    LOG(DEBUG);

    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> result = task->get_future();

    {
      Lock lock(mutex);
      if (stop) {
        LOG(FATAL) << "Attempted to add task to stopped threadpool";
        throw std::runtime_error("Threadpool is shut down");
      }
      tasks.emplace([task]() { (*task)(); });
    }

    LOG(DEBUG) << "added task; waking thread to run it";
    available.notify_one();
    return result;
  }

  void shutdown()
  {
    LOG(DEBUG);
    Lock lock(mutex);
    stop = true;
  }

  void join(bool shutdown = false)
  {
    if (shutdown)
      this->shutdown();

      LOG(DEBUG) << "joining " << threads.size() << " threads...";

      for (std::thread& thread : threads)
        if (thread.joinable())
          thread.join();
      LOG(DEBUG) << "... joined";
  }

  std::size_t nr_threads() const
  {
    return threads.size();
  }

  std::size_t size() const
  {
    return tasks.size();
  }

  Threadpool(Threadpool&& move)
    : stop(move.stop)
    , threads(std::move(move.threads))
    , tasks(std::move(move.tasks))
  {
  }

  Threadpool& operator=(Threadpool&& move)
  {
    stop = move.stop;
    std::swap(threads, move.threads);
    std::swap(tasks, move.tasks);
    return *this;
  }

private:
  std::vector<std::thread> threads;
  Container<std::function<void()>> tasks;

  std::mutex mutex;
  std::condition_variable available;
  volatile bool stop{false};
};
}
