#pragma once

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
  using taskid_type = std::size_t;
}


template <typename R>
class Task;


class Scheduler
{
  using Lock = std::unique_lock<std::mutex>;

protected:
  using millis = std::chrono::milliseconds;

public:
  template <typename F, typename... Args>
  auto do_async(F&& f, Args&&... args)
      -> std::future<typename std::result_of<F(Args...)>::type>
  {
    auto taskpair = std::move(make_task(f, args)... );
    do_async(std::move(taskpair.second));
    return std::move(taskpair.first);
  }

  template <typename Delay, typename F, typename... Args>
  auto schedule(Delay delay, F&& f, Args&&... args)
      -> std::future<typename std::result_of<F(Args...)>::type>
  {
    auto taskpair = std::move(make_task(f, args)...);
    schedule(std::move(taskpair.second),
             std::chrono::duration_cast<std::chrono::milliseconds>(delay));
    return std::move(taskpair.first);
  }

protected:
  virtual void do_async(std::function<void()> task) = 0;
  virtual void schedule(std::function<void()> task, millis delay_ms) = 0;


private:
  // clang-format off
  template <typename F, typename... Args>
  auto make_task(F&& f, Args&&... args) ->
    std::pair<
      std::future<typename std::result_of<F(Args...)>::type>,
      std::function<void()>
    >
  {
    using R = typename std::result_of<F(Args...)>::type;
    auto proxy =
      std::make_shared<std::packaged_task<R()>>(
          std::bind(
            std::forward<F>(f),
            std::forward<Args>(args)...
          )
      );
    // clang-format on

    auto runner = [proxy]() { (*proxy)(); };

    return std::make_pair(std::move(proxy->get_future()), std::move(runner));
  }


  std::mutex mutex;

public:
  class Factory
  {
  public:
    virtual std::unique_ptr<Scheduler> new_scheduler() const = 0;
  };
};
}
