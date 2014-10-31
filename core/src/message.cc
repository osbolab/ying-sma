#include "message.hh"


namespace sma
{

Message::Message()
{
}
Message::Message(std::size_t len)
  : dataBytes(len)
{
}
Message::Message(const std::vector<std::uint8_t>& copy)
  : dataBytes(copy)
{
}
Message::Message(std::vector<std::uint8_t>&& move)
  : dataBytes(std::move(move))
{
}

bool Message::operator==(const Message& other) const
{
  return dataBytes == other.dataBytes;
}
bool Message::operator!=(const Message& other) const
{
  return !(*this == other);
}

std::vector<std::uint8_t>& Message::data()
{
  return dataBytes;
}

const std::vector<std::uint8_t>& Message::cdata() const
{
  return dataBytes;
}
}
