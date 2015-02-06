#pragma once

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentmetadata.hpp>
#include <sma/util/serial.hpp>

#include <iosfwd>
#include <cstdint>
#include <string>

namespace sma
{
struct Interest {
  TRIVIALLY_SERIALIZABLE(Interest, type)

  Interest(ContentType type)
    : type(type)
  {
  }

  Interest(ContentMetadata const& metadata)
    : Interest(metadata.type)
  {
  }

  bool operator==(Interest const& rhs) const { return type == rhs.type; }
  bool operator!=(Interest const& rhs) const { return !(*this == rhs); }
  bool operator<(Interest const& rhs) const { return type < rhs.type; }
  bool operator>(Interest const& rhs) const { return type > rhs.type; }
  bool operator<=(Interest const& rhs) const { return type <= rhs.type; }
  bool operator>=(Interest const& rhs) const { return type >= rhs.type; }

  explicit operator std::string() const { return std::string(type); }

private:
  friend struct std::hash<Interest>;

  /****************************************************************************
   * Serialized Fields - Order matters!
   */
  ContentType type;
  /***************************************************************************/
};

inline std::ostream& operator<<(std::ostream& os, Interest const& interest)
{
  return os << std::string(interest);
}
}

namespace std
{
template <>
struct hash<sma::Interest> {
  using argument_type = sma::Interest;
  using result_type = std::size_t;

  result_type operator()(argument_type const& a) const
  {
    return hash<sma::ContentType>()(a.type);
  }
};
}
