#pragma once

#include <sma/neighborhelper.hpp>
#include <sma/neighbortable.hpp>

#include <sma/smafwd.hpp>

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

  void receive(MessageHeader header, Beacon msg) override;

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
