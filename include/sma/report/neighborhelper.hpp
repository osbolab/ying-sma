#pragma once

#include <sma/report/report.hpp>

namespace sma
{
struct NodeId;
class NeighborHelper;

template <>
class Report<NeighborHelper>
{
public:
  static void add_neighbor(NeighborHelper* h, NodeId node);
  static void drop_neighbor(NeighborHelper* h, NodeId node);

private:
  Report<NeighborHelper>();
  Report<NeighborHelper>(NeighborHelper&&);
  void operator=(NeighborHelper&&);
};
}
