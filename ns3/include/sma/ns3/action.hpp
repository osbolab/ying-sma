#pragma once

#include <ns3/core-module.h>

#include <memory>
#include <deque>
#include <chrono>

namespace sma
{
class CcnNode;
class Ns3NodeContainer;

struct Action {
  using nanos = std::chrono::nanoseconds;
  using delay_unit = nanos;

  template <typename Duration>
  static nanos delay_to_absolute(Duration const& delay)
  {
    return nanos(ns3::Simulator::Now().GetNanoSeconds()) + delay;
  }

  Action(Ns3NodeContainer& context)
    : context(&context)
  {
  }

  virtual ~Action() { ns3::Simulator::Cancel(event_id); }

  virtual void operator()() = 0;

  nanos remaining_delay_ns()
  {
    auto delay = time - nanos(ns3::Simulator::Now().GetNanoSeconds());
    return (delay.count() >= 0) ? delay : nanos(0);
  }

  Ns3NodeContainer* context;
  /*! Absolute time at which this action will execute. */
  nanos time;

  ns3::EventId event_id;
};
}
