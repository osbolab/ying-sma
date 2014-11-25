#include <sma/ns3/async.hpp>
#include <sma/io/log>

#include <ns3/simulator.h>
#include <ns3/nstime.h>

#include <functional>


namespace sma
{
namespace detail
{
  struct Ns3AsyncProxy {
    void operator()()
    {
      if (target)
        target();
      delete this;
    }
    std::function<void()> target;
  };
}

void Ns3Async::schedule(Actor* caller,
                        std::function<void()> f,
                        std::chrono::nanoseconds delay)
{
  // Unfortunately we can't just use the function pointer directly
  // because ns3 doesn't support std::function, so we have to wrap it in
  // yet another layer of indirection.
  // This one stores a heap object that delegates to the task function
  // so the simulator has something tangible to call into with its C function
  // pointer.
  // The heap proxy object deletes itself when it's called. No biggie.
  std::vector<ns3::EventId>* events;

  auto it = scheduled.find(caller);
  if (it == scheduled.end()) {
    auto ins = scheduled.emplace(caller, std::vector<ns3::EventId>());
    events = &(ins.first->second);
  } else

    events = &(it->second);

  events->emplace_back(
      ns3::Simulator::Schedule(ns3::NanoSeconds(delay.count()),
                               &detail::Ns3AsyncProxy::operator(),
                               new detail::Ns3AsyncProxy{f}));
  while (events->size() > 100)
    events->erase(events->begin(), events->begin() + 50);
}

void Ns3Async::purge_events_for(Actor* actor)
{
  auto it = scheduled.find(actor);
  if (it == scheduled.end())
    return;
  auto& events = it->second;
  for (auto& event : events)
    ns3::Simulator::Cancel(event);
  scheduled.erase(it);
}
}
