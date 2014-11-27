#pragma once

#include <chrono>
#include <memory>
#include <future>
#include <cassert>
#include <utility>
#include <functional>
#include <type_traits>

namespace sma
{
template <typename F, typename... A>
struct Task {
  using result_type = typename std::result_of<F(A...)>::type;

  Task(F&& f, A&&... args)
    : task(std::make_shared<std::packaged_task<R()>>(
          std::bind(std::forward<F>(f), std::forward<A>(args)...)))
  {
  }

  Task(Task&& t)
    : task(std::move(t.task))
  {
  }

  Task& operator=(Task&& t)
  {
    std::swap(task, t.task);
    return *this;
  }

  template <typename Delay>
  std::future<result_type> do_in(Delay delay);
  std::future<result_type> do_now()
  {
    return do_in(std::chrono::nanoseconds(0));
  }

private:
  std::shared_ptr<std::packaged_task<R()>> task;
};

template <typename F, typename... A>
template <typename Delay>
std::future<typename Task<F, A...>::R> Task<F, A...>::do_in(Delay delay)
{
  auto task = std::move(this->task);
  assert(task);
  auto fut = task->get_future();
  async::schedule([=]() { (*task)(); },
                  std::chrono::duration_cast<std::chrono::nanoseconds>(delay));
  return fut;
}
}
