#include <utility>
#include <vector>

#include "msg/Message.hh"

namespace sma
{

using std::size_t;
using std::uint8_t;
using std::vector;

Message::Message()
{
}

Message::Message(const uint8_t* data, size_t len)
  : data(vector<const uint8_t>(data, data + len))
{
}

Message::Message(const Message& copy)
  : data(vector<const uint8_t>(copy.data))
{
}

Message::Message(Message&& move)
{
  std::swap(data, move.data);
}

Message& Message::operator=(const Message& copy)
{
  data = vector<const uint8_t>(copy.data);
  return *this;
}

Message& Message::operator=(Message&& move)
{
  std::swap(data, move.data);
  return *this;
}

const vector<const uint8_t>& Message::getData() const
{
  return data;
}

bool Message::operator ==(const Message& other) const
{
  return data == other.data;
}

bool Message::operator !=(const Message& other) const
{
  return !(*this == other);
}

}