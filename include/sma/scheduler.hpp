#pragma once

#include <sma/log.hpp>

#include <memory>
#include <future>
#include <utility>
#include <functional>
#include <type_traits>


namespace sma
{

class scheduler
{
  using Lock = std::unique_lock<std::mutex>;

protected:
  using millis = std::chrono::milliseconds;

public:
  virtual ~scheduler() {}

  template <typename Delay, typename F, typename... Args>
  auto schedule(Delay delay, F&& f, Args&&... args)
      -> std::future<typename std::result_of<F(Args...)>::type>
  {
    using R = typename std::result_of<F(Args...)>::type;

    auto proxy = std::make_shared<std::packaged_task<R()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    schedule([=]() { (*proxy)(); }, std::chrono::duration_cast<millis>(delay));

    return proxy->get_future();
  }

protected:
  scheduler() {}

  // The implementer should do the actual scheduling here
  virtual void schedule(std::function<void()>, millis delay) = 0;
};
}
