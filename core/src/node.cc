#include "node.hh"
#include "bytebuf.hh"


namespace sma
{

const std::size_t Node::Id::size = sizeof(Node::Id::data);
inline Node::~Node() {}

inline bytebuf& operator<<(bytebuf& dst, Node::Id id)
{
  return dst.put(id.data, Node::Id::size);
}
inline bytebuf& operator>>(bytebuf& src, Node::Id& id)
{
  operator>>(static_cast<bytebuf::view&>(src), id);
  return src;
}
inline bytebuf::view& operator>>(bytebuf::view& src, Node::Id& id)
{
  src.get(id.data, Node::Id::size);
  return src;
}
}
