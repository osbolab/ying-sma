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

class scheduler
{
  using Lock = std::unique_lock<std::mutex>;

protected:
  using millis = std::chrono::milliseconds;

public:
  ~scheduler() { LOG(DEBUG); }

  template <typename Delay, typename F, typename... Args>
  auto schedule(Delay delay, F&& f, Args&&... args)
      -> std::future<typename std::result_of<F(Args...)>::type>
  {
    LOG(DEBUG);
    using R = typename std::result_of<F(Args...)>::type;

    auto proxy = std::make_shared<std::packaged_task<R()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    schedule([=]() { (*proxy)(); }, to_millis(delay));

    return proxy->get_future();
  }

protected:
  // The implementer should do the actual scheduling here
  virtual void schedule(std::function<void()>, millis delay) = 0;

private:
  std::mutex mutex;

public:
  class factory
  {
  public:
    ~factory() {}

    virtual std::unique_ptr<scheduler> new_scheduler() const = 0;
  };
};
}
