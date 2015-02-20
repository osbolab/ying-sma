#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <sma/async.hpp>

#include <sma/io/log.hpp>


namespace sma
{
void Async::schedule(std::function<void()> f, std::chrono::nanoseconds delay)
{
  Logger("Async").d("schedule in %v ns", delay.count());
}

void Async::purge()
{
}
}
