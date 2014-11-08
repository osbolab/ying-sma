#include "message.hh"
#include "bytes.hh"

#include <cassert>
#include <utility>
#include <vector>


namespace sma
{

Message::Message(Type type,
                 ActorId sender,
                 std::vector<ActorId> recipients)
  : type_(type)
  , sender_(sender)
  , recipients_(recipients)
{
}

Message::Message(Message&& m)
  : type_(m.type_)
  , sender_(m.sender_)
{
  m.type_ = 0;
  m.sender_ = ActorId {{0}};
  std::swap(recipients_, m.recipients_);
}

std::size_t Message::write_fields(std::uint8_t* dst, std::size_t len) const
{
  std::uint8_t* const dst_start = dst;
  dst += Bytes::put(dst, type_);
  dst += Bytes::put(dst, sender_.id, sizeof(ActorId::id));
  dst += Bytes::put(dst, static_cast<field_nr_recp_type>(recipients_.size()));
  for (auto& recp : recipients_)
    dst += Bytes::put(dst, recp.id, sizeof(ActorId::id));

  return dst - dst_start;
}

std::size_t Message::read_fields(const std::uint8_t* src, std::size_t len)
{
  const std::uint8_t* const src_start = src;

  return src - src_start;
}

Message& Message::operator=(Message&& m)
{
  std::swap(type_, m.type_);
  std::swap(sender_, m.sender_);
  std::swap(recipients_, m.recipients_);
  return *this;
}

bool Message::operator==(const Message& other) const
{
  return true;
}

}
