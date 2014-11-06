#pragma once

#include "log.hh"

#include <cstdlib>
#include <thread>
#include <future>
#include <functional>
#include <vector>
#include <mutex>
#include <atomic>
#include <memory>
#include <condition_variable>
#include <queue>


namespace sma
{

class Threadpool
{
  using Lock = std::unique_lock<std::mutex>;

public:
  Threadpool(std::size_t nr_core_threads)
    : state(new shared_state(nr_core_threads))
  {
    for (std::size_t i = 0; i < nr_core_threads; ++i)
      add_thread();
  }

  Threadpool(Threadpool&& o)
    : state(std::move(o.state))
  {
  }

  ~Threadpool() { shutdown(); }

  Threadpool& operator=(Threadpool&& o)
  {
    std::swap(state, o.state);
    return *this;
  }

  template <typename F, typename... Args>
  auto push(F&& f, Args&&... args)
      -> std::future<typename std::result_of<F(Args...)>::type>;

  void shutdown();
  void join();


private:
  void add_thread();

  struct shared_state {
    shared_state(std::size_t nr)
      : nr_core_threads(nr)
    {
    }

    const std::size_t nr_core_threads;
    std::vector<std::thread> threads;
    std::vector<std::function<void()>> tasks;
    std::mutex tasks_mutex;
    std::mutex threads_mutex;
    std::condition_variable available;
    bool stop{false};
  };
  std::unique_ptr<shared_state> state;
};


template <typename F, typename... Args>
auto Threadpool::push(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{

  using return_type = typename std::result_of<F(Args...)>::type;

  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> result = task->get_future();

  {
    Lock lock_tasks{state->tasks_mutex};
    if (state->stop)
      throw std::runtime_error("Threadpool is shut down");
    state->tasks.emplace_back([task]() { (*task)(); });
  }

  {
    Lock lock_threads{state->threads_mutex};
    if (state->threads.size() < state->tasks.size())
      add_thread();
  }

  state->available.notify_one();
  return result;
}
}
