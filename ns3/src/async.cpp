#include <sma/async>
#include <sma/log>

#include <ns3/simulator.h>
#include <ns3/nstime.h>

#include <functional>


namespace sma
{
namespace detail
{
  struct ns3_async_proxy {
    void operator()()
    {
      if (target)
        target();
      delete this;
    }
    std::function<void()> target;
  };

  void async_scheduler::schedule(std::function<void()> f,
                                 std::chrono::nanoseconds delay)
  {
    // Unfortunately we can't just use the function pointer directly
    // because ns3 doesn't support std::function, so we have to wrap it in
    // yet another layer of indirection.
    // This one stores a heap object that delegates to the task function
    // so the simulator has something tangible to call into with its C function
    // pointer.
    // The heap proxy object deletes itself when it's called. No biggie.
    ns3::Simulator::Schedule(ns3::NanoSeconds(delay.count()),
                             &ns3_async_proxy::operator(),
                             new ns3_async_proxy{f});
  }
}
}
