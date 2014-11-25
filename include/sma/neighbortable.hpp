#pragma once

#include <sma/nodeid.hpp>
#include <sma/neighbor.hpp>

#include <vector>
#include <unordered_map>

namespace sma
{
class NeighborTable
{
public:
  Neighbor update(NodeId node);
  void prune(std::vector<Neighbor>* optional_pruned_out);

private:
  static constexpr auto DEFAULT_MAX_AGE = std::chrono::milliseconds(10000);

  std::unordered_map<NodeId, Neighbor> table;
};
}
