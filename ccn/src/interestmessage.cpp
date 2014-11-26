#include <sma/ccn/interestmessage.hpp>
#include <sma/message.hpp>

#include <sma/binaryformatter.hpp>
#include <sma/bytearraywriter.hpp>
#include <sma/bytearrayreader.hpp>
#include <sma/serial/vector.hpp>

namespace sma
{
constexpr decltype(InterestMessage::TYPE) InterestMessage::TYPE;

InterestMessage InterestMessage::read(std::uint8_t const* src, std::size_t size)
{
  auto reader = ByteArrayReader(src, size).format<BinaryFormatter>();
  return InterestMessage(&reader);
}

InterestMessage::InterestMessage(NodeId sender, value_type interest)
  : sender(sender)
{
  interests.push_back(std::move(interest));
}
InterestMessage::InterestMessage(NodeId sender, interest_vector interests)
  : sender(sender)
  , interests(std::move(interests))
{
  assert(interests.size() < std::numeric_limits<count_type>::max());
}

Message InterestMessage::make_message() const
{
  ByteArrayWriter buf;
  buf.format<BinaryFormatter>() << *this;
  // FIXME: this is seriously broken as it copies twice: buf.data() has to copy
  // the dynamic array.
  return Message::copy(
      InterestMessage::TYPE, Message::LIGHT, buf.data(), buf.size());
}
}
