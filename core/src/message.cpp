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
message::message(message&& r)
  : sender_(std::move(r.sender_))
  , recipients_(std::move(r.recipients_))
  , domain_(std::move(r.domain_))
  , id_(std::move(r.id_))
  , content_(std::move(r.content_))
  , content_len_(std::move(r.content_len_))
{
}
message::message(const message& r)
  : sender_(r.sender_)
  , recipients_(r.recipients_)
  , domain_(r.domain_)
  , id_(r.id_)
  , content_len_(r.content_len_)
  , content_(r.content_)
{
}

pinned_message message::pin() const
{
  return pinned_message(*this);
}

pinned_message::pinned_message(const message& r)
  : message(r)
  , pinned(pin_copy(r.content_, r.content_len_))
{
  content_ = pinned.get();
}
pinned_message::pinned_message(const pinned_message& r)
  : pinned_message(static_cast<const message&>(r))
{
}
pinned_message::pinned_message(pinned_message&& r)
  : message(r)
  , pinned(std::move(r.pinned))
{
  content_ = pinned.get();
}

message& message::operator=(message&& r)
{
  std::swap(sender_, r.sender_);
  std::swap(recipients_, r.recipients_);
  std::swap(domain_, r.domain_);
  std::swap(id_, r.id_);
  std::swap(content_len_, r.content_len_);
  std::swap(content_, r.content_);
  return *this;
}
message& message::operator=(const message& r)
{
  sender_ = r.sender_;
  recipients_ = r.recipients_;
  domain_ = r.domain_;
  id_ = r.id_;
  content_len_ = r.content_len_;
  content_ = r.content_;
  return *this;
}
/* message - Regular Construction
 *****************************************************************************/

std::unique_ptr<std::uint8_t[]> pinned_message::pin_copy(const void* src,
                                                  std::size_t len)
{
  auto pin = std::make_unique<std::uint8_t[]>(len);
  std::memcpy(pin.get(), src, len);
  return pin;
}
}
