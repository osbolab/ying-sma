#pragma once

#include <sma/buffer.hpp>

#include <cstdint>
#include <iostream>
#include <iomanip>


namespace sma
{

class node
{
public:
  struct id {
    std::uint8_t data[2];
    static const std::size_t size;
  };

  virtual ~node() = 0;
};

inline node::~node() {}

inline buffer& operator<<(buffer& dst, const node::id& id)
{
  return dst.put(id.data, node::id::size);
}
inline buffer& operator>>(buffer& src, node::id& id)
{
  src.get(id.data, node::id::size);
  return src;
}
inline buffer::view& operator>>(buffer::view& src, node::id& id)
{
  src.get(id.data, node::id::size);
  return src;
}

inline std::ostream& operator<<(std::ostream& os, const node::id& id)
{
  os << std::hex << std::setfill('0') << std::nouppercase;
  for (std::size_t i = 0; i < node::id::size; ++i) {
    os << std::setw(2) << std::uint32_t{id.data[i]};
    if (i + 1 < node::id::size)
      os << ':';
  }
  os << std::dec << std::setfill(' ');
  return os;
}
}
