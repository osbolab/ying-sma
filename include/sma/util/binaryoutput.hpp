#pragma once

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
  BinaryOutput(std::ostream& os)
    : os(&os)
  {
  }

  BinaryOutput(BinaryOutput const& r)
    : os(r.os)
  {
  }

  BinaryOutput& operator=(BinaryOutput const& r)
  {
    os = r.os;
    return *this;
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
    operator<<(std::uint8_t(v.size()));
    for (auto const& t : v)
      operator<<(t);
    return *this;
  }

  // clang-format off
  BinaryOutput& operator<< (bool const& t);
  BinaryOutput& operator<< (std::string const& t);
  BinaryOutput& operator<< (float const& t);
  BinaryOutput& operator<< (double const& t);
  BinaryOutput& operator<< (std::uint8_t const& t);
  BinaryOutput& operator<< (std::uint16_t const& t);
  BinaryOutput& operator<< (std::uint32_t const& t);
  BinaryOutput& operator<< (std::uint64_t const& t);

  BinaryOutput& operator<< (std::int8_t const& t);
  BinaryOutput& operator<< (std::int16_t const& t);
  BinaryOutput& operator<< (std::int32_t const& t);
  BinaryOutput& operator<< (std::int64_t const& t);
  // clang-format on
};
}
