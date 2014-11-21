#include <sma/binarymessagewriter.hpp>

namespace sma
{
BinaryMessageWriter::BinaryMessageWriter(void* dst, std::size_t size)
  : dst(dst, size)
{
}
BinaryMessageWriter::BinaryMessageWriter(std::size_t size)
  : dst(size)
{
}

std::size_t serialized_size(Message const& msg)
{
  return sizeof(Message::Type) + sizeof(Message::data_size_type)
         + msg.data_size();
}

std::size_t BinaryMessageWriter::write(Message const& msg)
{
  std::size_t const size = serialized_size(msg);
  assert(dst.remaining() >= size);
  dst << msg.type();
  dst << msg.data_size();
  dst.put(msg.cdata(), msg.data_size());
  return size;
}
}
