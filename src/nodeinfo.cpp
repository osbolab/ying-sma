#include <sma/nodeinfo.hpp>

#include <ostream>

namespace sma
{
std::ostream& operator<<(std::ostream& os, NodeInfo const& node)
{
  return os << "node(" << node.id() << ")";
}
}
