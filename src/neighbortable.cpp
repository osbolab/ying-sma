#include <sma/neighbortable.hpp>

namespace sma
{
constexpr decltype(
    NeighborTable::DEFAULT_MAX_AGE) NeighborTable::DEFAULT_MAX_AGE;

Neighbor NeighborTable::update(NodeId node)
{
  auto it = table.find(node);
  if (it == table.end())
    return table.emplace(node, node).first->second;
  it->second.touch();
  return it->second;
}

void NeighborTable::prune(std::vector<Neighbor>* optional_pruned_out) {}
}
