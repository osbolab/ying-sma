#include <sma/message.hpp>
#include <sma/node.hpp>
#include <sma/buffer.hpp>

#include <cstdint>
#include <cassert>
#include <utility>
#include <vector>
#include <iostream>


namespace sma
{

// clang-format off
std::ostream& operator<<(std::ostream& os, const message::stub& stb)
{
  os << "partial-message {\n   domain: " << std::size_t{stb.domain}
     << "\n , recipients: [";
      for (auto& recp : stb.recipients) os << recp << ", ";
  os << "]\n , content: (" << stb.len << " bytes)\n}";
  return os;
}

std::ostream& operator<<(std::ostream& os, const message& msg)
{
  os << "message {\n   domain: " << std::size_t{msg.domain_}
     << "\n , id: " << std::size_t{msg.id_}
     << "\n , sender: " << msg.sender_
     << "\n , recipients: [";
      for (auto& recp : msg.recipients_) os << recp << ", ";
  os << "]\n , content: (" << msg.content_len_ << " bytes)\n}";
  return os;
}
// clang-format on

/******************************************************************************
 * message stub
 */
message::stub::stub(domain_type domain)
  : domain(domain)
{
}
message::stub& message::stub::add_recipient(node::id recipient)
{
  recipients.push_back(recipient);
  return *this;
}
message::stub& message::stub::wrap_contents(const std::uint8_t* src,
                                            std::size_t len)
{
  content = src;
  this->len = len;
  return *this;
}
message message::stub::build(node::id sender, id_type id) const
{
  return message(std::move(sender), std::move(id), *this);
}

/* message - Deserialization/Construction
******************************************************************************/


/******************************************************************************
 * message - Deserialization/Construction
 */
message::message(buffer::view src)
{
  src >> sender_;

  field_nrecp_type nrecipients;
  src >> nrecipients;
  recipients_.resize(nrecipients);
  for (std::size_t i = 0; i < nrecipients; ++i)
    src >> recipients_[i];

  src >> domain_;
  src >> id_;

  field_len_type len;
  src >> len;
  content_len_ = static_cast<std::size_t>(len);
  content_ = src.cbuf();
}
message::message(const std::uint8_t* src, std::size_t len)
  : message(buffer::view::of(src, len))
{
}
/* message - Deserialization/Construction
******************************************************************************/

/******************************************************************************
 * message - Serialization
 */
std::size_t message::serialize_to(buffer dst) const
{
  assert(dst.remaining() >= serial_size());
  std::size_t start = dst.position();
  dst << sender_;
  dst << static_cast<field_nrecp_type>(recipients_.size());
  for (auto& recp : recipients_)
    dst << recp;
  dst << domain_;
  dst << id_;
  dst << static_cast<field_len_type>(content_len_);
  dst << arrcopy(content_, content_len_);
  return dst.position() - start;
}
std::size_t message::serialize_to(std::uint8_t* dst, std::size_t len) const
{
  return serialize_to(buffer::wrap(dst, len));
}
std::size_t message::serial_size() const
{
  // clang-format off
  return node::id::size
       + sizeof(field_nrecp_type)
       + node::id::size * recipients_.size()
       + sizeof(domain_type)
       + sizeof(field_len_type)
       + content_len_;
  // clang-format on
}
/* message - Serialization
******************************************************************************/

/******************************************************************************
 * message - Regular Constructon
 */
message::message(node::id sender, id_type id, const stub& stb)
  : domain_(stb.domain)
  , sender_(sender)
  , recipients_(stb.recipients)
  , id_(id)
  , content_(stb.content)
  , content_len_(stb.len)
{
}
message::message(message&& rhs)
  : sender_(std::move(rhs.sender_))
  , recipients_(std::move(rhs.recipients_))
  , domain_(std::move(rhs.domain_))
  , id_(std::move(rhs.id_))
  , content_(std::move(rhs.content_))
  , content_len_(std::move(rhs.content_len_))
  , pinned(std::move(rhs.pinned))
{
}
message::message(const message& rhs)
  : sender_(rhs.sender_)
  , recipients_(rhs.recipients_)
  , domain_(rhs.domain_)
  , id_(rhs.id_)
  , content_len_(rhs.content_len_)
{
  if (!rhs.pinned)
    throw_not_pinned();
  content_ = pin_copy(rhs.content_, content_len_);
}

message& message::operator=(message&& rhs)
{
  std::swap(sender_, rhs.sender_);
  std::swap(recipients_, rhs.recipients_);
  std::swap(domain_, rhs.domain_);
  std::swap(id_, rhs.id_);
  std::swap(content_, rhs.content_);
  std::swap(content_len_, rhs.content_len_);
  std::swap(pinned, rhs.pinned);
  return *this;
}
message& message::operator=(const message& rhs)
{
  if (!rhs.pinned)
    throw_not_pinned();

  sender_ = rhs.sender_;
  recipients_ = rhs.recipients_;
  domain_ = rhs.domain_;
  id_ = rhs.id_;
  content_len_= rhs.content_len_;
  content_ = pin_copy(rhs.content_, content_len_);
  return *this;
}
/* message - Regular Construction
 *****************************************************************************/

std::uint8_t* message::pin_copy(const void* src, std::size_t len)
{
  pinned = std::make_unique<std::uint8_t[]>(len);
  std::memcpy(pinned.get(), src, len);
  content_ = pinned.get();
  return pinned.get();
}

std::uint8_t* message::pin()
{
  if (!pinned) {
    return pin_copy(content_, content_len_);
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
