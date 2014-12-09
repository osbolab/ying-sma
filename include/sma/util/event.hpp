#pragma once

#include <mutex>
#include <vector>
#include <functional>

namespace sma
{
template <typename... Args>
class Event
{
  using Myt = Event<Args...>;

  using Lock = std::lock_guard<std::mutex>;

public:
  using handler = std::function<bool(Args...)>;

  Event() {}
  Event(Myt const&) = delete;
  Myt& operator=(Myt const&) = delete;

  Myt& operator+=(handler h)
  {
    Lock lock(mx);
    handlers.push_back(std::move(h));
    return *this;
  }

  void operator()(Args&&... args)
  {
    Lock lock(mx);
    auto it = handlers.begin();
    while (it != handlers.end())
      if (!*it(std::forward<Args>(args)...))
        it = handlers.erase(it);
      else
        ++it;
  }

private:
  std::mutex mx;
  std::vector<handler> handlers;
};
}
