#pragma once

#include <cstdint>
#include <istream>
#include <sstream>

namespace sma
{
class ByteArrayReader
{
public:
  ByteArrayReader(std::uint8_t const* src, std::size_t size);

  template <template <typename> class Formatter>
  Formatter<std::istream> format()
  {
    return Formatter<std::istream>(&is);
  }

private:
  std::stringbuf sb;
  std::istream is;
};
}
