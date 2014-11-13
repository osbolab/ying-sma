#include <sma/msg/message.hpp>
#include <sma/node.hpp>
#include <sma/byte_buf.hpp>

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
  src >> sender_;

  const auto nrecipients = src.get<field_nrecp_type>();
  recipients_.resize(nrecipients);
  for (std::size_t i = 0; i < nrecipients; ++i)
    src >> recipients_[i];

  src >> domain_;
  src >> id_;

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
  dst << sender_;
  dst << static_cast<field_nrecp_type>(recipients_.size());
  for (auto& recp : recipients_)
    dst << recp;
  dst << domain_;
  dst << id_;
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
       + node::id::size * recipients_.size()
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
message::message(node::id sender, id_type id, const builder& bld)
  : domain_(bld.domain)
  , sender_(sender)
  , recipients_(bld.recipients)
  , id_(id)
  , data(bld.data)
  , len(bld.len)
{
}
message::message(message&& rhs)
  : sender_(std::move(rhs.sender_))
  , recipients_(std::move(rhs.recipients_))
  , domain_(std::move(rhs.domain_))
  , id_(std::move(rhs.id_))
  , data(std::move(rhs.data))
  , len(std::move(rhs.len))
  , pinned(std::move(rhs.pinned))
{
}
message::message(const message& rhs)
  : sender_(rhs.sender_)
  , recipients_(rhs.recipients_)
  , domain_(rhs.domain_)
  , id_(rhs.id_)
  , len(rhs.len)
{
  if (!rhs.pinned)
    throw_not_pinned();
  data = pin_copy(rhs.data, len);
}

message& message::operator=(message&& rhs)
{
  std::swap(sender_, rhs.sender_);
  std::swap(recipients_, rhs.recipients_);
  std::swap(domain_, rhs.domain_);
  std::swap(id_, rhs.id_);
  std::swap(data, rhs.data);
  std::swap(len, rhs.len);
  std::swap(pinned, rhs.pinned);
  return *this;
}
message& message::operator=(const message& rhs)
{
  if (!rhs.pinned)
    throw_not_pinned();

  sender_ = rhs.sender_;
  recipients_ = rhs.recipients_;
  domain_= rhs.domain_;
  id_ = rhs.id_;
  len = rhs.len;
  data = pin_copy(rhs.data, len);
  return *this;
}
/* message - Regular Construction
 *****************************************************************************/

std::uint8_t* message::pin_copy(const void* src, std::size_t len)
{
  pinned = std::make_unique<std::uint8_t[]>(len);
  std::memcpy(pinned.get(), src, len);
  data = pinned.get();
  return pinned.get();
}
std::uint8_t* message::pin()
{
  if (!pinned) {
    return pin_copy(data, len);
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
