#pragma once

#include <sma/util/serial.hpp>

#include <iosfwd>
#include <cstdint>
#include <string>

namespace sma
{
struct ContentType {
  TRIVIALLY_SERIALIZABLE(ContentType, value)

  ContentType(std::string value)
    : value(value)
  {
  }

  bool operator==(ContentType const& r) const { return value == r.value; }
  bool operator!=(ContentType const& r) const { return value != r.value; }
  bool operator<(ContentType const& r) const { return value < r.value; }
  bool operator>(ContentType const& r) const { return value > r.value; }
  bool operator<=(ContentType const& r) const { return value <= r.value; }
  bool operator>=(ContentType const& r) const { return value >= r.value; }

  explicit operator std::string() const { return value; }

private:
  friend struct std::hash<ContentType>;

  /****************************************************************************
   * Serialized Fields - Order matters!
   */
  std::string value;
  /***************************************************************************/
};

inline std::ostream& operator<<(std::ostream& os, ContentType const& value)
{
  return os << std::string(value);
}
}

namespace std
{
template <>
struct hash<sma::ContentType> {
  using argument_type = sma::ContentType;
  using result_type = std::size_t;

  result_type operator()(argument_type const& a) const
  {
    return hash<std::string>()(a.value);
  }
};
}
