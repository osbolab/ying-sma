#pragma once

#include <sma/neighbors.hpp>
#include <sma/neighbortable.hpp>

#include <sma/io/log>

#include <sma/chrono.hpp>
#include <chrono>

namespace sma
{
class CcnNode;

struct MessageHeader;
struct NeighborMessage;

class NeighborHelper : public Neighbors
{
public:
  NeighborHelper(CcnNode& node);

  void receive(MessageHeader&& header, NeighborMessage&& msg);

private:
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  std::chrono::milliseconds prune_interval{1000};

  void schedule_beacon(std::chrono::milliseconds delay);
  void beacon();
  void prune_neighbors();

  time_point next_beacon_time;

  CcnNode* node;
  Logger log;

  NeighborTable neighbors;
};
}
