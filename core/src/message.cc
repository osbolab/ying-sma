#include "message.hh"
#include "node.hh"
#include "bytebuffer.hh"

#include <cassert>
#include <utility>
#include <vector>


namespace sma
{

Message::Message(Type type,
                 Node::Id sender,
                 std::vector<Node::Id> recipients,
                 ByteView body)
  : type_(std::move(type))
  , sender_(std::move(sender))
  , recipients_(std::move(recipients))
  , body(std::move(body))
{
}

Message::Message(const ByteView& csrc)
{
  auto src = csrc.view();

  sender_ = src.get<Node::Id>();

  auto nrecipients = src.get<field_nrecp>();
  recipients_.reserve(nrecipients);
  for (std::size_t i = 0; i < nrecipients; ++i)
    recipients_.emplace_back(src.get<Node::Id>());

  type_ = src.get<decltype(type_)>();

  auto body_len = static_cast<std::size_t>(src.get<field_body_len>());
  body = src.limit(body_len);
}

inline ByteBuffer& operator<<(ByteBuffer& dst, Node::Id id)
{
  dst.put(id.data, Node::Id::size);
  return dst;
}

std::size_t Message::put_in(ByteBuffer& dst) const
{
  std::size_t start = dst.position();
  dst.put(sender_.data, Node::Id::size);
  dst << static_cast<field_nrecp>(recipients_.size());
  for (auto& recp : recipients_)
    dst.put(recp.data, Node::Id::size);
  dst << type_;
  return dst.position() - start;
}

void Message::get_fields(ByteView& src)
{
  src.get(sender_.data, Node::Id::size);
  field_nrecp nr_recipients;
  src >> nr_recipients;
  recipients_ = std::vector<Node::Id>(nr_recipients);
  for (std::size_t i = 0; i < nr_recipients; ++i)
    src.get(recipients_[i].data, Node::Id::size);
  src >> type_;
}

bool Message::operator==(const Message& other) const { return true; }
}
