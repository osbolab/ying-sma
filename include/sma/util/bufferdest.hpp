#pragma once

#include <cstdint>
#include <ostream>
#include <sstream>
#include <memory>

namespace sma
{
class BufferDest
{
public:
  BufferDest();
  BufferDest(std::size_t size);
  BufferDest(std::uint8_t* dst, std::size_t size);

  template <typename Formatter>
  Formatter format()
  {
    return Formatter(&os);
  }

  std::size_t read(std::uint8_t* dst, std::size_t size);
  std::uint8_t* data();
  std::size_t size();

private:
  char* buf;
  std::stringbuf sb;
  std::ostream os;
  std::unique_ptr<char[]> owned;
  bool dynamic{false};
};
}
