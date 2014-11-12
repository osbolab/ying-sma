#pragma once

#include <sma/log.hpp>

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

template <typename R>
class Task final
{
  friend class scheduler;

  using Myt = Task<R>;

  using Lock = std::unique_lock<std::mutex>;

public:
  ~Task() { LOG(DEBUG); }

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


class scheduler
{
  using Lock = std::unique_lock<std::mutex>;

protected:
  using millis = std::chrono::milliseconds;

public:
  ~scheduler() { LOG(DEBUG); }

  template <typename Delay, typename F, typename... Args>
  auto schedule(Delay delay, F&& f, Args&&... args)
      -> std::shared_ptr<Task<typename std::result_of<F(Args...)>::type>>
  {
    LOG(DEBUG);
    using R = typename std::result_of<F(Args...)>::type;

    auto proxy = std::make_shared<std::packaged_task<R()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    auto task = std::shared_ptr<Task<R>>(new Task<R>{proxy->get_future()});

    auto runner = [proxy, task]() {
      LOG(DEBUG) << "<task runner proxy>";
      (*proxy)();
    };
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
    ~Factory() {}

    virtual std::unique_ptr<scheduler> new_scheduler() const = 0;
  };
};
}
