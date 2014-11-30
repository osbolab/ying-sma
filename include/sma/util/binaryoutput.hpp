#pragma once

#include <sma/util/detail/uint_with_size.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <ostream>

namespace sma
{
class BinaryOutput
{
  std::ostream* os;

public:
  BinaryOutput(std::ostream* os)
    : os(os)
  {
  }

  BinaryOutput& write(void const* src, std::size_t size)
  {
    os->write(reinterpret_cast<char const*>(src), size);
    return *this;
  }

  template <typename T>
  BinaryOutput& operator<<(T const& t)
  {
    t.write_fields(*this);
    return *this;
  }

  template <typename T>
  BinaryOutput& operator<<(std::vector<T> const& v)
  {
    *this << std::uint8_t(v.size());
    for (auto& t : v)
      *this << t;
    return *this;
  }

  // clang-format off
  BinaryOutput& operator<< (std::string const& t);
  BinaryOutput& operator<< (float const& t);
  BinaryOutput& operator<< (double const& t);
  BinaryOutput& operator<< (std::uint8_t const& t);
  BinaryOutput& operator<< (std::uint16_t const& t);
  BinaryOutput& operator<< (std::uint32_t const& t);
  BinaryOutput& operator<< (std::uint64_t const& t);
  // clang-format on
};
}
