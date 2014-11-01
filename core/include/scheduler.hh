#pragma once

#include "chrono_literals.hh"

#include <mutex>
#include <memory>
#include <future>
#include <utility>
#include <functional>
#include <type_traits>
#include <unordered_map>



namespace sma
{

namespace detail
{
  using taskid_type = std::size_t;
}

template <typename F, typename... Args>
class Task final
{
  friend class Scheduler;

  using Myt = Task<F, Args...>;
  using R = typename std::result_of<F(Args...)>::type;

  using Lock = std::unique_lock<std::mutex>;

public:
  Task(Myt&& move) = default;
  Myt& operator=(Myt&& move) = default;

  R wait()
  {
    {
      Lock lock{mutex};
      if (!ftr->valid())
        throw std::future_error{std::future_errc::no_state};
      if (is_set)
        return ftr->get();
      return ftr->wait();
    }
  }

  bool poll(R& out)
  {
    {
      Lock lock{mutex};
      if (!ftr->valid())
        throw std::future_error{std::future_errc::no_state};
      if (!is_set)
        return false;
    }
    out = ftr->get();
    return true;
  }

  bool cancel()
  {
    Lock lock{mutex};
    if (!cancellable())
      return false;
    cancel_requested = true;
    return true;
  }

  bool cancellable()
  {
    Lock lock{mutex};
    return !is_done && !is_started && !is_cancelled;
  }

  bool started()
  {
    Lock lock{mutex};
    return is_started;
  }

  bool done()
  {
    Lock lock{mutex};
    return is_done;
  }

  bool cancelled()
  {
    Lock lock{mutex};
    return is_cancelled;
  }

private:
  Task(F&& f, Args&&... args)
  {
    // clang-format off

    // The functon takes arguments, but it would be annoying to store those
    // and pass them at the call site--plus we would need to paramterize
    // everything to store them. So:
    //  1. bind the arguments to the function in a wrapper.
    //  2. store that wrapper in an R() proxy that stores the return
    //     value with a future<R> to get it.
    //  3. capture the future<R> so we can get the return value after calling
    //     the proxy.
    //  4. store a void() lambda so we can call it from an unparameterized
    //     container (a container that stores void())

    // The R of R(void) in steps 2 and 3 above
    using R = typename std::result_of<F(Args...)>::type;

    // Step 2:
    // Manage the wrapper with a proxy callable with no arguments.
    // The proxy captures the return value so we can retrieve it later.
    // Must be a shared pointer because we're wrapping it in a void() lambda,
    // but packaged_task is uncopyable.
    auto proxy =
      std::make_shared<std::packaged_task<R()>>(
          // Step 1: bind the provided arguments to the target function
          std::bind(
            std::forward<F>(f),
            std::forward<Args>(args)...
          )
      );

    // Step 3:
    // Steal the future from the proxy so we can provide it to the Task's user
    // This future gets the return value stored by the proxy.
    ftr = proxy->get_future();

    // Step 4:
    // Wrap the whole thing in a void() so we can store it in unparameterized
    // containers.
    runner =
      // Copy the proxy pointer to move the proxy out of this scope
      [proxy, this]()
      {
        // Calling this void() calls the proxy and sets the future<R>
        // with the return value of the target function.
        (*proxy)();
        complete();
      };

    // clang-format on
  }

  // Called by the embedded void() lambda after the proxy is called;
  // signals that the task has run and the future is ready.
  void complete()
  {
    Lock lock{mutex};
    is_done = true;
  }

  Task(const Myt& copy) = delete;
  Myt& operator=(const Myt& copy) = delete;

  detail::taskid_type id;
  std::function<void()> runner;
  std::future<R> ftr;

  std::mutex mutex;

  bool is_set{false};
  bool is_started{false};
  bool is_done{false};
  bool cancel_requested{false};
  bool is_cancelled{false};
};


class Scheduler
{
  using Lock = std::unique_lock<std::mutex>;

protected:
  using millis = std::chrono::milliseconds;

public:
  ~Scheduler()
  {
  }

  template <typename Delay, typename F, typename... Args>
  auto schedule(Delay delay, F&& f, Args&&... args)
      -> std::shared_ptr<Task<F, Args...>>
  {
    // clang-format off
    auto task = new Task<F, Args...>{
                   std::forward<F>(f),
                   std::forward<Args>(args)...
                 };
    // clang-format on

    {
      Lock lock(mutex);
      tasks.insert(std::make_pair(task->id, &(task->runner)));
    }

    schedule(task->id, to_millis(delay));

    return std::shared_ptr<Task<F, Args...>>{task};
  }

protected:
  // Implement to call run(taskid_type) when delay is elapsed.
  virtual void schedule(std::size_t runnerid, millis delay) = 0;
  // Call when a scheduled task's delay has elapsed.
  // The task is run in the calling thread.
  void run(detail::taskid_type taskid);

private:
  std::mutex mutex;
  std::unordered_map<detail::taskid_type, std::function<void()>*> tasks;


public:
  class Factory
  {
  public:
    ~Factory()
    {
    }

    virtual std::unique_ptr<Scheduler> new_scheduler() const = 0;
  };
};
}
