#include <sma/ccn/contenttype.hpp>

#include <ostream>

namespace sma
{
std::ostream& operator<<(std::ostream& os, ContentType const& value)
{
  return os << std::string(value);
}
}
