#pragma once

#include "log.hh"

#include <cstdlib>
#include <vector>
#include <thread>
#include <functional>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>


namespace sma
{

class Threadpool
{
public:
  using Task = std::function<void()>;

protected:
  struct shared_state {
    shared_state(std::size_t min_capacity)
      : min_capacity(min_capacity)
      , tasks(min_capacity)
    {
    }

    std::atomic_bool stop{false};
    std::mutex mx;
    std::condition_variable available;

    std::size_t min_capacity;
    std::vector<Task> tasks;
    std::size_t p{0};
    std::size_t t{0};
    std::size_t count{0};

    std::atomic_flag resizing = ATOMIC_FLAG_INIT;
  };

public:
  Threadpool(std::size_t nr_threads);
  Threadpool(std::size_t nr_threads, std::size_t initial_capacity);
  Threadpool(const Threadpool& rhs) = delete;
  Threadpool& operator=(const Threadpool& rhs) = delete;
  virtual ~Threadpool();

  void push_back(Task task);
  void join();

protected:
  virtual void thread_body(shared_state& s);

private:
  static std::size_t resize(shared_state& s, std::size_t len);
  template <typename T>
  static void ringmove(T dst, T src, std::size_t src_off, std::size_t len);

  std::vector<std::thread> threads;
  std::unique_ptr<shared_state> s;
};

template <typename T>
void Threadpool::ringmove(T dst, T src, std::size_t off, std::size_t len)
{
  const std::size_t sz = src.size();
  // Copy the old ring into the front of the new one
  for (std::size_t d = 0; d < len; ++d) {
    dst[d] = std::move(src[off++]);
    if (off == sz)
      off = 0;
  }
}

#if 0
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

  std::size_t nr_threads() const { return state->threads.size(); }

  std::size_t size() const { return state->tasks.size(); }

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
#endif
}
