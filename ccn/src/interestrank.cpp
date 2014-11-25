#include <sma/ccn/interestrank.hpp>

#include <ostream>

namespace sma
{
std::ostream& operator<<(std::ostream& os, InterestRank const& value)
{
  return os << std::string(value);
}
}
