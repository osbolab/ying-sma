
#pragma once

#include <mutex>
#include <memory>
#include <future>
#include <utility>
#include <functional>
#include <type_traits>



namespace sma
{

namespace detail
{

struct Task_ final {
  // The task target binding
  std::function<void()> f;
  // Called by the scheduler at the end of execution to update the task's
  // state
  std::function<void()> completer;

  Task_()
  {
  }

  Task_(std::function<void()>&& f, std::function<void()>&& completer)
    : f(std::move(f))
    , completer(std::move(completer))
  {
  }
};
}

template <typename T>
class Task final
{
  friend class Scheduler;

  using Myt = Task<T>;
  using Lock = std::unique_lock<std::mutex>;

public:
  Task(Myt&& move) = default;
  Myt& operator=(Myt&& move) = default;

  T wait()
  {
    {
      Lock lock{mutex};
      if (!ftr->valid()) throw std::future_error{std::future_errc::no_state};
      if (is_set) return ftr->get();
      return ftr->wait();
    }
  }

  bool poll(T& out)
  {
    {
      Lock lock{mutex};
      if (!ftr->valid()) throw std::future_error{std::future_errc::no_state};
      if (!is_set) return false;
    }
    out = ftr->get();
    return true;
  }

  bool cancel()
  {
    Lock lock{mutex};
    if (!cancellable()) return false;
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
  template <typename F, typename... Args>
  Task(F&& f, Args&&... args)
  {
    // Deduce the type of the function created by f(args) so we can pass it to
    // bind without specifying it
    using return_type = typename std::result_of<F(Args...)>::type;
    static_assert(std::is_same<T, return_type>::value,
                  "Task template type T must be the same as function return type");

    // Bind f(args) in an asynchronous task runner in a copyable pointer
    auto binding = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    ftr = std::make_shared<std::future<return_type>>(std::move(binding->get_future()));

    task_ = detail::Task_{[binding]() { (*binding)(); }, [this]() { complete(); }};
  }


  void complete()
  {
    Lock lock{mutex};
    is_done = true;
  }


  Task(const Myt& copy) = delete;
  Myt& operator=(const Myt& copy) = delete;



  detail::Task_ task_;
  std::shared_ptr<std::future<T>> ftr;

  std::mutex mutex;

  bool is_set{false};
  bool is_started{false};
  bool is_done{false};
  bool cancel_requested{false};
  bool is_cancelled{false};
};



class Scheduler final
{
public:
  template <typename Delay, typename F, typename... Args>
  auto schedule(Delay delay, F&& f, Args&&... args)
      -> std::shared_ptr<Task<typename std::result_of<F(Args...)>::type>>
  {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = new Task<return_type>{std::forward<F>(f), std::forward<Args>(args)...};
    return std::shared_ptr<Task<return_type>>{task};
  }
};
}
