#pragma once

#include <sma/util/reader.hpp>

#include <cstdint>
#include <istream>
#include <sstream>

namespace sma
{
class BufferSource
{
public:
  BufferSource(char const* src, std::size_t size);
  BufferSource(std::uint8_t const* src, std::size_t size);

  void rewind();

  template <typename Formatter>
  Reader<Formatter> format()
  {
    return Reader<Formatter>(is);
  }

private:
  std::stringbuf sb;
  std::istream is;
};
}
