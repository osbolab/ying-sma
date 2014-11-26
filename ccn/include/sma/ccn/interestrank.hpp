#pragma once

#include <iosfwd>
#include <cstdint>
#include <string>
#include <type_traits>

namespace sma
{
struct InterestRank {
  InterestRank() : value(0) {}

  template <
      typename T,
      typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
  InterestRank(T t)
    : value(t)
  {
  }
  bool operator==(InterestRank const& r) const { return value == r.value; }
  bool operator!=(InterestRank const& r) const { return value != r.value; }
  bool operator<=(InterestRank const& r) const { return value <= r.value; }
  bool operator>=(InterestRank const& r) const { return value >= r.value; }
  bool operator<(InterestRank const& r) const { return value < r.value; }
  bool operator>(InterestRank const& r) const { return value > r.value; }

  explicit operator std::uint32_t() const { return value; }
  explicit operator std::string() const { return std::to_string(value); }

private:
  friend struct std::hash<InterestRank>;

  std::uint32_t value;
};

std::ostream& operator<<(std::ostream& os, InterestRank const& value);
}

namespace std
{
template <>
struct hash<sma::InterestRank> {
  using argument_type = sma::InterestRank;
  using result_type = std::size_t;

  result_type operator()(argument_type const& a) const { return a.value; }
};
}
