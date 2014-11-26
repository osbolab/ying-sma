#pragma once

#include <sma/nodeid.hpp>
#include <sma/neighbor.hpp>

#include <vector>
#include <unordered_map>

namespace sma
{
class NeighborTable
{
  using table_type = std::unordered_map<NodeId, Neighbor>;

public:
  using value_type = table_type::value_type;
  using iterator = table_type::iterator;
  using const_iterator = table_type::const_iterator;
  using size_type = table_type::size_type;

  Neighbor update(NodeId node);
  void prune(std::chrono::milliseconds max_age,
             std::vector<value_type>* optional_pruned_out = nullptr);

  size_type size() const { return table.size(); }
  bool empty() const { return table.empty(); }
  iterator begin() { return table.begin(); }
  iterator end() { return table.end(); }
  const_iterator cbegin() const { return table.cbegin(); }
  const_iterator cend() const { return table.cend(); }

private:
  std::unordered_map<NodeId, Neighbor> table;
};
}
