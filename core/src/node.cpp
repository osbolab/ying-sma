#include <sma/node.hpp>
#include <sma/byte_buf.hpp>


namespace sma
{

const std::size_t node::id::size = sizeof(node::id::data);
inline node::~node() {}

inline byte_buf& operator<<(byte_buf& dst, node::id id)
{
  return dst.put(id.data, node::id::size);
}
inline byte_buf& operator>>(byte_buf& src, node::id& id)
{
  operator>>(static_cast<byte_buf::view&>(src), id);
  return src;
}
inline byte_buf::view& operator>>(byte_buf::view& src, node::id& id)
{
  src.get(id.data, node::id::size);
  return src;
}
}
