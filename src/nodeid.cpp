#include <sma/nodeid.hpp>

#include <ostream>

namespace sma
{
std::ostream& operator<<(std::ostream& os, NodeId const& value)
{
  return os << std::string(value);
}
}
