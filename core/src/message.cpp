#include <sma/core/message.hpp>
#include <sma/core/node.hpp>
#include <sma/core/byte_buf.hpp>

#include <cassert>
#include <utility>
#include <vector>


namespace sma
{

/******************************************************************************
 * message - Deserialization/Construction
 */
message::message(byte_buf::view src)
{
  src >> senderid;

  const auto nrecipients = src.get<field_nrecp_type>();
  recpids.resize(nrecipients);
  for (std::size_t i = 0; i < nrecipients; ++i)
    src >> recpids[i];

  src >> dom;
  src >> id;

  len = static_cast<std::size_t>(src.get<field_len_type>());
  data = src.cbuf();
}
message::message(const std::uint8_t* src, std::size_t len)
  : message(byte_buf::view::of(src, len))
{
}
/* message - Deserialization/Construction
******************************************************************************/

/******************************************************************************
 * message - Serialization
 */
std::size_t message::copy_to(byte_buf dst) const
{
  assert(dst.remaining() >= total_len());
  std::size_t start = dst.position();
  dst << senderid;
  dst << static_cast<field_nrecp_type>(recpids.size());
  for (auto& recp : recpids)
    dst << recp;
  dst << dom;
  dst << id;
  dst << static_cast<field_len_type>(len);
  dst << arrcopy(data, len);
  return dst.position() - start;
}
std::size_t message::copy_to(std::uint8_t* dst, std::size_t len) const
{
  return copy_to(byte_buf::wrap(dst, len));
}
std::size_t message::total_len() const
{
  // clang-format off
  return node::id::size
       + sizeof(field_nrecp_type)
       + node::id::size * recpids.size()
       + sizeof(domain_type)
       + sizeof(field_len_type)
       + len;
  // clang-format on
}
/* message - Serialization
******************************************************************************/

/******************************************************************************
 * message - Regular Constructon
 */
message::message(domain_type domain,
                 node::id sender,
                 std::vector<node::id> recipients,
                 const std::uint8_t* content,
                 std::size_t len)
  : dom(std::move(domain))
  , senderid(std::move(sender))
  , recpids(std::move(recipients))
  , data(content)
  , len(len)
{
}

message::message(message&& rhs)
  : senderid(std::move(rhs.senderid))
  , recpids(std::move(rhs.recpids))
  , dom(std::move(rhs.dom))
  , id(std::move(rhs.id))
  , data(std::move(rhs.data))
  , len(std::move(rhs.len))
  , pinned(std::move(rhs.pinned))
{
}
message::message(const message& rhs)
  : senderid(rhs.senderid)
  , recpids(rhs.recpids)
  , dom(rhs.dom)
  , id(rhs.id)
  , len(rhs.len)
{
  if (!rhs.pinned)
    throw_not_pinned();
  data = pin_copy_(rhs.data, len);
}

message& message::operator=(message&& rhs)
{
  std::swap(senderid, rhs.senderid);
  std::swap(recpids, rhs.recpids);
  std::swap(dom, rhs.dom);
  std::swap(id, rhs.id);
  std::swap(data, rhs.data);
  std::swap(len, rhs.len);
  std::swap(pinned, rhs.pinned);
  return *this;
}
message& message::operator=(const message& rhs)
{
  if (!rhs.pinned)
    throw_not_pinned();

  senderid = rhs.senderid;
  recpids = rhs.recpids;
  dom= rhs.dom;
  id = rhs.id;
  len = rhs.len;
  data = pin_copy_(rhs.data, len);
  return *this;
}
/* message - Regular Construction
 *****************************************************************************/

std::uint8_t* message::pin_copy_(const void* src, std::size_t len)
{
  pinned = std::make_unique<std::uint8_t[]>(len);
  std::memcpy(pinned.get(), src, len);
  data = pinned.get();
  return pinned.get();
}
std::uint8_t* message::pin()
{
  if (!pinned) {
    return pin_copy_(data, len);
  }
  return pinned.get();
}

void message::throw_not_pinned()
{
  throw std::runtime_error(
      "message was copied without being pinned first"
      " and illegally increased the visibility of unmanaged memory.");
}
}
