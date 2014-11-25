#pragma once

#include <sma/nodeid.hpp>

#include <iosfwd>

namespace sma
{
struct NodeInfo {
public:
  NodeInfo(NodeId id)
    : id_{id}
  {
  }

  NodeId id() const { return id_; }

private:
  NodeId id_;
};

std::ostream& operator<<(std::ostream& os, NodeInfo const& node);
}
