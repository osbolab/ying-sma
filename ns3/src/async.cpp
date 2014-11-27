#include <sma/async.hpp>

#include <ns3/simulator.h>
#include <ns3/nstime.h>

#include <unordered_set>
#include <functional>


namespace sma
{
std::unordered_set<ns3::EventId> events;

struct Ns3AsyncProxy {
  void operator()()
  {
    if (target)
      target();

    delete this;
  }

  ~Ns3AsyncProxy() { events.erase(id); }

  ns3::EventId id;
  std::function<void()> target;
};

void Async::schedule(std::function<void()> f, std::chrono::nanoseconds delay)
{
  // Unfortunately we can't just use the function pointer directly
  // because ns3 doesn't support std::function, so we have to wrap it in
  // yet another layer of indirection.
  // This one stores a heap object that delegates to the task function
  // so the simulator has something tangible to call into with its C function
  // pointer.
  // The heap proxy object deletes itself when it's called. No biggie.
  auto proxy = new Ns3AsyncProxy;
  proxy->target = f;
  proxy->id = ns3::Simulator::Schedule(
      ns3::NanoSeconds(delay.count()), &Ns3AsyncProxy::operator(), proxy);
  events->insert(proxy->id);
}

void Async::purge()
{
  for (auto& event : events)
    ns3::Simulator::Cancel(event);
  ns3::events.clear();
}
}
