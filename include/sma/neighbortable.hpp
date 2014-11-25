#pragma once

#include <sma/neighbor.hpp>

#include <vector>

namespace sma
{
class NeighborTable
{
public:
  Neighbor update(NodeId node);

private:
  std::vector<Neighbor> table;
};
}
