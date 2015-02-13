#include <sma/async.hpp>

#include <ns3/simulator.h>
#include <ns3/nstime.h>

#include <unordered_set>
#include <functional>
#include <atomic>


namespace std
{
  template<>
  struct hash<ns3::EventId> {
  using argument_type = ns3::EventId;
  using result_type = std::size_t;

  result_type operator()(argument_type const& a) const { return a.GetUid(); }
  };
}

namespace sma
{
std::unordered_set<ns3::EventId> ns3_events;
std::atomic_bool purged{false};

struct Ns3AsyncProxy {
  void operator()()
  {
    if (target)
      target();

    delete this;
  }

  ~Ns3AsyncProxy() { ns3_events.erase(id); }

  ns3::EventId id;
  std::function<void()> target;
};

void Async::schedule(std::function<void()> f, std::chrono::nanoseconds delay)
{
  if (purged)
    return;
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
  ns3_events.insert(proxy->id);
}

void Async::purge()
{
  purged = true;
  for (auto& event : ns3_events)
    ns3::Simulator::Cancel(event);
  ns3_events.clear();
}
}
