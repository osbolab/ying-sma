#include <sma/ns3/ns3_scheduler.hpp>
#include <sma/log>

#include <ns3/simulator.h>
#include <ns3/nstime.h>

#include <memory>
#include <functional>


namespace sma
{
ns3_scheduler::~ns3_scheduler() {}

void ns3_scheduler::schedule(std::function<void()> f, millis delay)
{
  // Unfortunately we can't just use the function pointer directly
  // because ns3 doesn't support std::function, so we have to wrap it in
  // yet another layer of indirection.
  // This one stores a heap object that delegates to the task function
  // so the simulator has something tangible to call into with its C function
  // pointer.
  // The heap proxy object deletes itself when it's called. No biggie.
  auto proxy = new wrapper{f};
  ns3::Simulator::Schedule(
      ns3::MilliSeconds(delay.count()), &wrapper::operator(), proxy);
}

void ns3_scheduler::wrapper::operator()()
{
  if (proxied)
    proxied();
  delete this;
}
}
