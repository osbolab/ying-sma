#pragma once

#include <sma/io/log>

#include <cassert>
#include <memory>
#include <utility>
#include <future>
#include <functional>
#include <type_traits>
#include <stdexcept>


namespace sma
{
class Async
{
  template <typename F, typename... A>
  friend class Task;

public:
  template <typename F, typename... A>
  class Task final
  {
  private:
    using R = typename std::result_of<F(A...)>::type;

  public:
    Task(Async* async, F&& f, A&&... args)
      : async(async)
      , task(std::make_shared<std::packaged_task<R()>>(
            std::bind(std::forward<F>(f), std::forward<A>(args)...)))
    {
    }
    Task(Task&& t)
      : async(t.async)
      , task(std::move(t.task))
    {
      t.async = nullptr;
    }
    Task& operator=(Task&& t)
    {
      std::swap(async, t.async);
      std::swap(task, t.task);
      return *this;
    }

    template <typename Delay>
    std::future<R> do_in(Delay delay);
    std::future<R> do_now() { return do_in(std::chrono::nanoseconds(0)); }

  private:
    Async* async;
    std::shared_ptr<std::packaged_task<R()>> task;
  };

  template <typename F, typename... A>
  Task<F, A...> make_task(F&& f, A&&... args)
  {
    return Task<F, A...>(this, std::forward<F>(f), std::forward<A>(args)...);
  }

  virtual ~Async() {}

protected:
  virtual void schedule(std::function<void()> f, std::chrono::nanoseconds delay)
      = 0;
};


template <typename F, typename... A>
template <typename Delay>
std::future<typename Async::Task<F, A...>::R>
Async::Task<F, A...>::do_in(Delay delay)
{
  auto task = std::move(this->task);
  assert(task);
  auto fut = task->get_future();
  async->schedule([=]() { (*task)(); },
                  std::chrono::duration_cast<std::chrono::nanoseconds>(delay));
  return fut;
}
}
