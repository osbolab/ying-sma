#include <sma/nodeinfo.hpp>


namespace sma
{
std::ostream& operator<<(std::ostream& os, NodeId const& id)
{
  os << id.id;
  return os;
}
std::ostream& operator<<(std::ostream& os, NodeInfo const& node)
{
  os << "node { \"id\": " << node.id() << " }";
  return os;
}
}
