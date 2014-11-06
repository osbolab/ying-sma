#pragma once

#include "chrono_literals.hh"

#include "log.hh"

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
  using taskid_type = size_t;
}

template <typename R>
class Task final
{
  friend class Scheduler;

  using Myt = Task<R>;

  using Lock = std::unique_lock<std::mutex>;

public:
  ~Task()
  {
    LOG(DEBUG);
  }

  // Implicitly delete the copy constructor and assignment operator
  Task(Myt&& move) = default;
  Myt& operator=(Myt&& move) = default;

  R wait()
  {
    LOG(DEBUG);
    {
      Lock lock{mutex};
      if (!ftr.valid())
        throw std::future_error{std::future_errc::no_state};
      if (is_set)
        return ftr.get();
      ftr.wait();
      return ftr.get();
    }
  }

  bool poll(R& out)
  {
    LOG(DEBUG);
    {
      Lock lock{mutex};
      if (!ftr.valid())
        throw std::future_error{std::future_errc::no_state};
      if (!is_set)
        return false;
    }
    out = ftr.get();
    return true;
  }

  bool cancel()
  {
    LOG(DEBUG);
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
  Task(std::future<R> ftr)
    : ftr(std::move(ftr))
  {
    LOG(DEBUG);
  }

  // Called by the embedded void() lambda after the proxy is called;
  // signals that the task has run and the future is ready.
  void complete()
  {
    LOG(DEBUG);
    Lock lock{mutex};
    is_done = true;
  }

  Task(const Myt& copy) = delete;
  Myt& operator=(const Myt& copy) = delete;

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
    LOG(DEBUG);
  }

  template <typename Delay, typename F, typename... Args>
  auto schedule(Delay delay, F&& f, Args&&... args)
      -> std::shared_ptr<Task<typename std::result_of<F(Args...)>::type>>
  {
    LOG(DEBUG);
    // clang-format off
    // The task takes arguments, but it would be annoying to store those
    // and pass them at the call site--plus we would need to paramterize
    // everything to store them. So:
    //  1. bind the arguments to the function in a wrapper.
    //  2. store that wrapper in an R() proxy that stores the return
    //     value with a future<R> to get it.
    //  3. capture the future<R> so we can get the return value after calling
    //     the proxy.
    //  4. store a void() lambda so we can call it from an unparameterized
    //     container (a container that stores void())

    // The R of R() in step 2
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
    auto task =
      std::shared_ptr<Task<R>>(
        new Task<R>{proxy->get_future()}
      );

    // Step 4:
    // Wrap the whole thing in a void() so we can store it in unparameterized
    // containers.
    auto runner =
      // Copy the proxy pointer to move the proxy out of this scope
      [proxy, task]()
      {
        LOG(DEBUG) << "<task runner proxy>";
        // Calling this void() calls the proxy and sets the future<R>
        // with the return value of the target function.
        (*proxy)();
        task->complete();
      };
    // clang-format on

    // Give the runner to the scheduler implementation so she can run the task
    // when ready. The runner implicitly updates the task, so the scheduler
    // implementation doesn't need to consider the type parameters that we do.
    schedule(std::move(runner), to_millis(delay));

    return task;
  }

protected:
  // The implementer should do the actual scheduling here
  virtual void schedule(std::function<void()>, millis delay) = 0;


private:
  std::mutex mutex;

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
