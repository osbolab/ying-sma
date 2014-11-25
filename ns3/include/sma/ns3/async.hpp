#pragma once

#include <sma/async.hpp>

#include <ns3/event-id.h>

#include <chrono>
#include <unordered_map>

namespace sma
{
class Actor;

class Ns3Async final : public Async
{
  template <typename F, typename... A>
  friend class Async::Task;

public:
  virtual ~Ns3Async() {}

protected:
  virtual void schedule(Actor* caller,
                        std::function<void()> f,
                        std::chrono::nanoseconds delay) override;
  virtual void purge_events_for(Actor* actor) override;

protected:
  std::unordered_map<Actor*, std::vector<ns3::EventId>> scheduled;
};
}
