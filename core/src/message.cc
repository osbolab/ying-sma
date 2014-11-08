#include "message.hh"
#include "bytebuffer.hh"

#include <cassert>
#include <utility>
#include <vector>


namespace sma
{

Message::Message(Type type, ActorId sender, std::vector<ActorId> recipients)
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
  m.sender_ = ActorId{{0}};
  std::swap(recipients_, m.recipients_);
}

inline ByteBuffer& operator<<(ByteBuffer& dst, ActorId id)
{
  dst.put(id.id, sizeof(ActorId::id));
  return dst;
}

std::size_t Message::put_in(ByteBuffer& dst) const
{
  std::size_t start = dst.position();
  dst.put(sender_.id, sizeof(ActorId::id));
  dst << static_cast<field_nr_recp_type>(recipients_.size());
  for (auto& recp : recipients_)
    dst.put(recp.id, sizeof(ActorId::id));
  dst << type_;
  return dst.position() - start;
}

void Message::get_fields(ByteView& src)
{
  src.get(sender_.id, sizeof(ActorId::id));
  field_nr_recp_type nr_recipients;
  src >> nr_recipients;
  recipients_ = std::vector<ActorId>(nr_recipients);
  for (std::size_t i = 0; i < nr_recipients; ++i)
    src.get(recipients_[i].id, sizeof(ActorId::id));
  src >> type_;
}

bool Message::operator==(const Message& other) const { return true; }
}
