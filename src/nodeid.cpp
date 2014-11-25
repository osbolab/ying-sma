#include <sma/nodeid.hpp>

#include <ostream>

namespace sma
{
std::ostream& operator<<(std::ostream& os, NodeId const& id)
{
  return os << std::string(id);
}
}
