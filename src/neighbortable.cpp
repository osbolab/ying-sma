#include <sma/neighbortable.hpp>

namespace sma
{
Neighbor NeighborTable::update(NodeId node)
{
  auto it = table.find(node);
  if (it == table.end())
    return table.emplace(std::move(node), Neighbor()).first->second;
  it->second.touch();
  return it->second;
}

void NeighborTable::prune(std::chrono::milliseconds max_age,
                          std::vector<value_type>* optional_pruned_out)
{
  auto it = table.begin();
  while (it != table.end())
    if (it->second.older_than(max_age)) {
      if (optional_pruned_out)
        optional_pruned_out->push_back(std::move(*it));
      it = table.erase(it);
    } else
      ++it;
}
}
