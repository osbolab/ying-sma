#include <sma/binarymessagewriter.hpp>

namespace sma
{
static constexpr std::size_t serialized_size(Message const& msg)
{
  return sizeof(Message::Type) + sizeof(Message::data_size_type)
         + msg.data_size();
}

std::size_t BinaryMessageWriter::write(std::uint8_t* dst,
                                       std::size_t size,
                                       Message const& msg)
{
  std::size_t const size_needed = serialized_size(msg);
  assert(size >= size_needed);
  auto buf = Buffer(dst, size);
  buf << msg.type() << msg.data_size();
  buf.put(msg.cdata(), msg.data_size());
  return size_needed;
}
}
