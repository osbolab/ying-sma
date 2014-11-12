#pragma once

#include <sma/byte_buf.hpp>

#include <cstdint>


namespace sma
{

class node
{
public:
  struct id {
    std::uint8_t data[2];
    static const std::size_t size;

    friend byte_buf& operator<<(byte_buf& dst, const id& id);
    friend byte_buf& operator>>(byte_buf& src, id& id);
    friend byte_buf::view& operator>>(byte_buf::view& src, id& id);
  };

  virtual ~node() = 0;
};
}
