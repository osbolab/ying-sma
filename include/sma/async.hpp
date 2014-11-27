#pragma once

#include <chrono>
#include <utility>
#include <functional>

namespace sma
{
struct Async {
  template <typename F, typename... A>
  static Task<F, A...> make_task(F&& f, A&&... args)
  {
    return Task<F, A...>(std::forward<F>(f), std::forward<A>(args)...);
  }

  // Defined in the platform-specific implementation
  static void purge();
  static void schedule(std::function<void()> f, std::chrono::nanoseconds delay);
};
}
