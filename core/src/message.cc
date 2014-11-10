#include "message.hh"
#include "node.hh"
#include "bytebuf.hh"

#include <cassert>
#include <utility>
#include <vector>


namespace sma
{

Message::Message(Type type,
                 Node::Id sender,
                 std::vector<Node::Id> recipients,
                 const std::uint8_t* content,
                 std::size_t len)
  : t(std::move(type))
  , senderid(std::move(sender))
  , recpids(std::move(recipients))
  , data(content)
  , len(len)
{
}

Message::Message(const std::uint8_t* src, std::size_t len)
{
  src >> senderid;

  const auto nrecipients = src.get<field_nrecp_type>();
  recpids.resize(nrecipients);
  for (std::size_t i = 0; i < nrecipients; ++i)
    src >> recpids[i];

  src >> t;

  len = static_cast<std::size_t>(src.get<field_len_type>());
}

std::size_t Message::write_to(bytebuf dst) const
{
  assert(dst.remaining() >= total_len());
  std::size_t start = dst.position();
  dst << senderid;
  dst << static_cast<field_nrecp_type>(recpids.size());
  for (auto& recp : recpids)
    dst << recp;
  dst << t;
  dst << static_cast<field_len_type>(len);
  dst << arrcopy(data, len);
  return dst.position() - start;
}

std::size_t Message::total_len() const
{
  // clang-format off
  return Node::Id::size
       + sizeof(field_nrecp_type)
       + Node::Id::size * recpids.size()
       + sizeof(Type)
       + sizeof(field_len_type)
       + len;
  // clang-format on
}

std::size_t Message::write_to(std::uint8_t* dst, std::size_t len) const
{
  return write_to(bytebuf::wrap(dst, len));
}
}
