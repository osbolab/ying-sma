#include <sma/neighbormessage.hpp>
#include <sma/message.hpp>

#include <sma/binaryformatter.hpp>
#include <sma/bytearraywriter.hpp>
#include <sma/bytearrayreader.hpp>

#include <utility>

namespace sma
{
constexpr decltype(NeighborMessage::TYPE) NeighborMessage::TYPE;

NeighborMessage NeighborMessage::read(std::uint8_t const* src, std::size_t size)
{
  auto reader = ByteArrayReader(src, size).format<BinaryFormatter>();
  return NeighborMessage(&reader);
}

NeighborMessage::NeighborMessage(NodeId sender)
  : sender(sender)
{
}

NeighborMessage::NeighborMessage(NodeId sender, body_type body)
  : sender(sender)
  , body(std::move(body))
{
}

NeighborMessage::NeighborMessage(NeighborMessage&& r)
  : sender(r.sender)
  , body(std::move(r.body))
{
}
NeighborMessage& NeighborMessage::operator=(NeighborMessage&& r)
{
  sender = r.sender;
  std::swap(body, r.body);
  return *this;
}

Message NeighborMessage::to_message() const
{
  ByteArrayWriter buf;
  buf.format<BinaryFormatter>() << *this;
  // FIXME: this is seriously broken as it copies twice: buf.data() has to copy
  // the dynamic array.
  return Message::copy(
      NeighborMessage::TYPE, Message::LIGHT, buf.data(), buf.size());
}
}
