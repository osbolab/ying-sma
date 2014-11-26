#include <sma/neighbormessage.hpp>
#include <sma/message.hpp>
#include <sma/forwardpolicy.hpp>

#include <sma/serial/binaryformatter.hpp>
#include <sma/util/buffersource.hpp>
#include <sma/util/bufferdest.hpp>

#include <utility>

namespace sma
{
constexpr decltype(NeighborMessage::TYPE) NeighborMessage::TYPE;

NeighborMessage NeighborMessage::read(std::uint8_t const* src, std::size_t size)
{
  auto reader = BufferSource(src, size).format<BinaryFormatter>();
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

Message NeighborMessage::make_message(NodeId sender) const
{
  BufferDest buf;
  buf.format<BinaryFormatter>() << *this;
  return Message(sender,
                 NeighborMessage::TYPE,
                 Message::body_type(buf),
                 ForwardPolicy{MessageFlow::LIGHT});
}
}
