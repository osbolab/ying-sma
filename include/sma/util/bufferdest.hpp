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
  BufferDest(char* dst, std::size_t size);
  BufferDest(std::uint8_t* dst, std::size_t size);

  template <typename Writer>
  Writer writer()
  {
    return Writer(os);
  }

  std::size_t size();
  void rewind();

private:
  char* buf;
  std::stringbuf sb;
  std::ostream os;
  std::unique_ptr<char[]> owned;
  bool dynamic{false};
};
}
