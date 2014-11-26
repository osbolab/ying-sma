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

Message::Message(NodeId sender, MessageType type, body_type body, ForwardPolicy policy)
  : sender(sender)
  , type(type)
  , body(std::move(body))
  , policy(policy)
{
}

Message::Message(Message const& r)
  : sender(r.sender)
  , recipients(r.recipients)
  , type(r.type)
  , body(r.body)
  , policy(r.policy)
{
}

Message::Message(Message&& r)
  : sender(std::move(r.sender))
  , recipients(std::move(r.recipients))
  , type(std::move(r.type))
  , body(std::move(r.body))
  , policy(std::move(r.policy))
{
}

Message& Message::operator=(Message&& r)
{
  sender = r.sender;
  type = r.type;
  std::swap(recipients, r.recipients);
  std::swap(body, r.body);
  std::swap(policy, r.policy);
  return *this;
}
}
