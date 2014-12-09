#pragma once

#include <sma/neighborhelper.hpp>
#include <sma/neighbortable.hpp>

#include <sma/smafwd.hpp>

#include <sma/util/event.hpp>

#include <sma/io/log>

#include <sma/chrono.hpp>
#include <chrono>

namespace sma
{
class CcnNode;

class NeighborHelperImpl : public NeighborHelper
{
public:
  NeighborHelperImpl(CcnNode& node);

  void saw(NodeId const& node) override;
  void saw(std::vector<NodeId> const& nodes) override;

  void receive(MessageHeader header, Beacon msg) override;

  //! Fired when new neighbors arrive.
  Event<std::vector<NodeId> const&> on_arrival;
  //! Fired when known neighbors are dropped due to inactivity.
  Event<std::vector<NodeId> const&> on_departure;

private:
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  std::chrono::milliseconds prune_interval{1000};

  void schedule_beacon(std::chrono::milliseconds delay);
  void beacon();
  void prune_neighbors();

  time_point next_beacon_time;

  NeighborTable neighbors;
};
}
