#pragma once

#include <cstdint>
#include <ostream>
#include <sstream>
#include <memory>

namespace sma
{
class ByteArrayWriter
{
public:
  ByteArrayWriter();
  ByteArrayWriter(std::size_t size);
  ByteArrayWriter(std::uint8_t* dst, std::size_t size);

  template <template <typename> class Formatter>
  Formatter<std::ostream> format()
  {
    return Formatter<std::ostream>(&os);
  }

  std::uint8_t* data();
  std::size_t size();

private:
  char* dst;
  std::stringbuf sb;
  std::ostream os;
  std::unique_ptr<char[]> owned;
  bool dynamic{false};
};
}
