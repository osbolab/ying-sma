#include <sma/message.hpp>

#include <cstdint>
#include <cassert>
#include <utility>
#include <vector>
#include <ostream>
#include <cstring>


namespace sma
{
// clang-format off
std::ostream& operator<<(std::ostream& os, Message const& m)
{
  os << "message[" << std::uint64_t(m.type) << "]: " << m.body.size() << " bytes";
  return os;
}
// clang-format on

Message::Message(MessageType type, NodeId sender, body_type body)
  : type(type)
  , sender(sender)
  , body(body)
{
}
}
