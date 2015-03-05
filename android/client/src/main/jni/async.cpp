#include <sma/async.hpp>
#include <sma/android/jniasynchelper.hpp>

#include <sma/io/log.hpp>


namespace sma
{
static Logger log("Async");

void Async::schedule(std::function<void()> f, std::chrono::nanoseconds delay)
{
  log.d("schedule in %v ns", delay.count());
  asynctaskqueue.emplace(delay, std::move(f));
  schedule_async_on_service_thread(delay);
}

void Async::purge()
{
  log.d("Purging async tasks");
  asynctaskqueue.clear();
}
}
