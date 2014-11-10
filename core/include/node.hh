#pragma once

#include "bytebuf.hh"

#include <cstdint>


namespace sma
{

class Node
{
public:
  struct Id {
    std::uint8_t data[2];
    static const std::size_t size;

    friend bytebuf& operator<<(bytebuf& dst, const Id& id);
    friend bytebuf& operator>>(bytebuf& src, Id& id);
    friend bytebuf::view& operator>>(bytebuf::view& src, Id& id);
  };

  virtual ~Node() = 0;
};

const std::size_t Node::Id::size = sizeof(Node::Id::data);
inline Node::~Node() {}
}
