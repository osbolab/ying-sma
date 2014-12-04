#pragma once

#include <sma/messagetypes.hpp>
#include <sma/messageheader.hpp>

#include <sma/util/binaryformat.hpp>
#include <sma/util/bufferdest.hpp>

namespace sma
{
struct Message {
  template <typename M>
  Message(MessageHeader header, M const& msg)
  {
    BufferDest dst(data, sizeof data);
    // clang-format off
    dst.format<BinaryOutput>()
      << header
      << MessageTypes::typecode<M>()
      << msg;
    // clang-format on
    sz = dst.size();
  }

  std::uint8_t const* cdata() { return data; }
  std::size_t size() { return sz; }

private:
  std::uint8_t data[20000];
  std::size_t sz;
};
}
