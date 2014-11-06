#pragma once

#include "log.hh"

#include <cstdlib>
#include <thread>
#include <future>
#include <functional>
#include <vector>
#include <mutex>
#include <memory>
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
    : state(new shared_state())
  {
    LOG(DEBUG) << nr_threads << " threads";

    // If we let the thread capture *this, then if this object gets moved
    // the thread will have invalid references to the members.
    shared_state* const cstate = state.get();

    for (std::size_t i = 0; i < nr_threads; ++i) {
      LOG(DEBUG) << "constructing " << i << "th thread";
      state->threads.emplace_back([cstate] {
        LOG(DEBUG) << "child thread spawned";
        for (;;) {
          std::function<void()> task;

          {
            Lock lock(cstate->mutex);
            if (cstate->tasks.empty()) {
              LOG(DEBUG) << "waiting for task";
              cstate->available.wait(lock, [cstate] {
                return cstate->stop || !cstate->tasks.empty();
              });
            }
            if (cstate->stop && cstate->tasks.empty())
              break;
            task = std::move(cstate->tasks.front());
            cstate->tasks.pop();
          }

          LOG(DEBUG) << "popped task; running";
          task();
        }
        LOG(DEBUG) << "child thread dead";
      });
      LOG(DEBUG) << state->threads.size() << " threads constructed";
    }
  }

  ~Threadpool()
  {
    if (state) {
      LOG(DEBUG) << "killing " << state->threads.size() << " threads";
      {
        Lock lock(state->mutex);
        state->stop = true;
      }
      state->available.notify_all();
      join();
    }
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
      Lock lock(state->mutex);
      if (state->stop) {
        LOG(FATAL) << "Attempted to add task to stopped threadpool";
        throw std::runtime_error("Threadpool is shut down");
      }
      state->tasks.emplace([task]() { (*task)(); });
    }

    LOG(DEBUG) << "added task; waking thread to run it";
    state->available.notify_one();
    return result;
  }

  void shutdown()
  {
    if (state) {
      LOG(DEBUG);
      Lock lock(state->mutex);
      state->stop = true;
    }
  }

  void join(bool shutdown = false)
  {
    if (state) {
      if (shutdown)
        this->shutdown();

      LOG(DEBUG) << "joining " << state->threads.size() << " threads...";

      for (std::thread& thread : state->threads)
        if (thread.joinable())
          thread.join();
      LOG(DEBUG) << "... joined";
    }
  }

  std::size_t nr_threads() const
  {
    return state->threads.size();
  }

  std::size_t size() const
  {
    return state->tasks.size();
  }

  Threadpool(Threadpool&& move)
    : state(std::move(move.state))
  {
  }

  Threadpool& operator=(Threadpool&& move)
  {
    std::swap(state, move.state);
    return *this;
  }

private:
  struct shared_state {
    std::vector<std::thread> threads;
    Container<std::function<void()>> tasks;
    std::mutex mutex;
    std::condition_variable available;
    bool stop{false};
  };
  std::unique_ptr<shared_state> state;
};
}
