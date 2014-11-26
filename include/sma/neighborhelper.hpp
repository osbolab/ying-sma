#pragma once

#include <sma/neighbortable.hpp>
#include <sma/neighbormessage.hpp>

#include <sma/io/log>

#include <sma/chrono>
#include <chrono>

namespace sma
{
class Node;
struct NeighborMessage;

class NeighborHelper
{
public:
  NeighborHelper(Node* node);

  void receive(NeighborMessage&& msg);
  void beacon_data(NeighborMessage::body_type data);

private:
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  std::chrono::milliseconds prune_interval{1000};

  void schedule_beacon(std::chrono::milliseconds delay);
  void beacon();
  void prune_neighbors();

  time_point next_beacon_time;
  bool have_beacon_data;
  NeighborMessage::body_type next_beacon_data;

  Node* node;
  Logger log;

  NeighborTable neighbors;
};
}
