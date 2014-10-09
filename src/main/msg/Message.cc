#include <utility>
#include <vector>

#include "msg/Message.hh"

using std::size_t;
using std::uint8_t;
using std::vector;

namespace sma
{

Message::Message(const uint8_t* data, size_t len)
  : data(vector<const uint8_t>(data, data + len)) {}

Message::Message(const Message& copy)
  : data(vector<const uint8_t>(copy.data)) {}

Message::Message(Message&& move) {
  std::swap(data, move.data);
}

Message& Message::operator=(Message copy) {
  std::swap(data, copy.data);
  return *this;
}

Message& Message::operator=(Message&& move) {
  std::swap(data, move.data);
  return *this;
}

const vector<const uint8_t>& Message::getData() const {
  return data;
}

}