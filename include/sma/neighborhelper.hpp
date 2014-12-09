#pragma once

#include <sma/helper.hpp>
#include <sma/smafwd.hpp>

#include <sma/io/log>

#include <vector>

namespace sma
{
class NeighborHelper : public Helper
{
public:
  NeighborHelper(CcnNode& node);

  virtual ~NeighborHelper() {}

  //! Update the neighbor table to include the specified node.
  virtual void saw(NodeId const& node) = 0;
  //! Update the neighbor table to include the specified nodes.
  virtual void saw(std::vector<NodeId> const& nodes) = 0;

  //! Handle an incoming beacon from a neighbor.
  virtual void receive(MessageHeader header, Beacon msg) = 0;
};
}
