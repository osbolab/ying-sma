#pragma once

#include <iosfwd>
#include <cstdint>
#include <string>
#include <type_traits>

namespace sma
{
struct NodeId {
private:
  using value_type = std::uint16_t;

public:
  template <
      typename T,
      typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
  NodeId(T value)
    : value{value}
  {
  }

  template <typename Reader>
  NodeId(Reader* r)
    : value{r->template get<value_type>()}
  {
  }

  template <typename Writer>
  void write_fields(Writer* w) const
  {
    *w << value;
  }

  operator std::uint64_t() const { return value; }
  operator std::uint32_t() const { return value; }
  operator std::uint16_t() const { return value; }
  operator std::string() const { return std::to_string(value); }

private:
  value_type value;
};

std::ostream& operator<<(std::ostream& os, NodeId const& value);
}
