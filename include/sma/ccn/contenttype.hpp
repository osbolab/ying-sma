#pragma once

#include <sma/util/reader.hpp>

#include <iosfwd>
#include <cstdint>
#include <string>

namespace sma
{
struct ContentType {
  ContentType(std::string name)
    : name(name)
  {
  }

  template <typename... T>
  ContentType(Reader<T...>& r)
    : name{r.template get<std::string>()}
  {
  }

  template <typename Writer>
  void write_fields(Writer& w) const
  {
    w << name;
  }

  bool operator==(ContentType const& r) const { return name == r.name; }
  bool operator!=(ContentType const& r) const { return name != r.name; }
  bool operator<(ContentType const& r) const { return name < r.name; }
  bool operator>(ContentType const& r) const { return name > r.name; }
  bool operator<=(ContentType const& r) const { return name <= r.name; }
  bool operator>=(ContentType const& r) const { return name >= r.name; }

  explicit operator std::string() const { return name; }

private:
  friend struct std::hash<ContentType>;

  /****************************************************************************
   * Serialized Fields - Order matters!
   */
  std::string name;
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
    return hash<std::string>()(a.name);
  }
};
}
