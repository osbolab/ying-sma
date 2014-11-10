#include "message.hh"
#include "node.hh"
#include "bytebuf.hh"

#include <cassert>
#include <utility>
#include <vector>


namespace sma
{

/******************************************************************************
 * Message - Deserialization/Construction
 */
Message::Message(bytebuf::view src)
{
  src >> senderid;

  const auto nrecipients = src.get<field_nrecp_type>();
  recpids.resize(nrecipients);
  for (std::size_t i = 0; i < nrecipients; ++i)
    src >> recpids[i];

  src >> t;
  src >> id;

  len = static_cast<std::size_t>(src.get<field_len_type>());
  data = src.cbuf();
}
Message::Message(const std::uint8_t* src, std::size_t len)
  : Message(bytebuf::view::of(src, len))
{
}
/* Message - Deserialization/Construction
******************************************************************************/

/******************************************************************************
 * Message - Serialization
 */
std::size_t Message::copy_to(bytebuf dst) const
{
  assert(dst.remaining() >= total_len());
  std::size_t start = dst.position();
  dst << senderid;
  dst << static_cast<field_nrecp_type>(recpids.size());
  for (auto& recp : recpids)
    dst << recp;
  dst << t;
  dst << id;
  dst << static_cast<field_len_type>(len);
  dst << arrcopy(data, len);
  return dst.position() - start;
}
std::size_t Message::copy_to(std::uint8_t* dst, std::size_t len) const
{
  return copy_to(bytebuf::wrap(dst, len));
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
/* Message - Serialization
******************************************************************************/

/******************************************************************************
 * Message - Regular Constructon
 */
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

Message::Message(Message&& rhs)
  : senderid(std::move(rhs.senderid))
  , recpids(std::move(rhs.recpids))
  , t(std::move(rhs.t))
  , id(std::move(rhs.id))
  , data(std::move(rhs.data))
  , len(std::move(rhs.len))
  , pinned(std::move(rhs.pinned))
{
}
Message::Message(const Message& rhs)
  : senderid(rhs.senderid)
  , recpids(rhs.recpids)
  , t(rhs.t)
  , id(rhs.id)
  , len(rhs.len)
{
  if (!rhs.pinned)
    throw_not_pinned();
  data = pin_copy_(rhs.data, len);
}

Message& Message::operator=(Message&& rhs)
{
  std::swap(senderid, rhs.senderid);
  std::swap(recpids, rhs.recpids);
  std::swap(t, rhs.t);
  std::swap(id, rhs.id);
  std::swap(data, rhs.data);
  std::swap(len, rhs.len);
  std::swap(pinned, rhs.pinned);
  return *this;
}
Message& Message::operator=(const Message& rhs)
{
  if (!rhs.pinned)
    throw_not_pinned();

  senderid = rhs.senderid;
  recpids = rhs.recpids;
  t = rhs.t;
  id = rhs.id;
  len = rhs.len;
  data = pin_copy_(rhs.data, len);
  return *this;
}
/* Message - Regular Construction
 *****************************************************************************/

std::uint8_t* Message::pin_copy_(const void* src, std::size_t len)
{
  pinned = std::make_unique<std::uint8_t[]>(len);
  std::memcpy(pinned.get(), src, len);
  data = pinned.get();
  return pinned.get();
}
std::uint8_t* Message::pin()
{
  if (!pinned) {
    return pin_copy_(data, len);
  }
  return pinned.get();
}

void Message::throw_not_pinned()
{
  throw std::runtime_error(
      "Message was copied without being pinned first"
      " and illegally increased the visibility of unmanaged memory.");
}
}
