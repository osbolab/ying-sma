#pragma once

#include <sma/log>

#include <cassert>
#include <memory>
#include <utility>
#include <future>
#include <functional>
#include <type_traits>
#include <stdexcept>


namespace sma
{

namespace detail
{
  class async_scheduler final
  {
    template<typename F, typename... Args>
    friend class async_task;

  public:
    static async_scheduler& instance()
    {
      static async_scheduler instance;
      return instance;
    }

  private:
    async_scheduler(){};
    async_scheduler(async_scheduler const&);
    void operator=(async_scheduler const&);

    void schedule(std::function<void()> f, std::chrono::nanoseconds delay);
  };


  template <typename F, typename... Args>
  class async_task final
  {
  private:
    using R = typename std::result_of<F(Args...)>::type;

  public:
    async_task(F&& f, Args&&... args)
      : task(std::make_shared<std::packaged_task<R()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)))
    {
    }

    template <typename Delay>
    std::future<R> do_in(Delay delay)
    {
      auto task = std::move(this->task);
      assert(task);
      auto fut = task->get_future();
      async_scheduler::instance().schedule(
          [=]() { (*task)(); },
          std::chrono::duration_cast<std::chrono::nanoseconds>(delay));
      return fut;
    }

  private:
    std::shared_ptr<std::packaged_task<R()>> task;
  };
}


template <typename F, typename... Args>
detail::async_task<F, Args...> async(F&& f, Args&&... args)
{
  return detail::async_task<F, Args...>(std::forward<F>(f),
                                     std::forward<Args>(args)...);
}
}
