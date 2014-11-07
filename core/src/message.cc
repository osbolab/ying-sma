#include "message.hh"

#include <cassert>
#include <cstdint>
#include <utility>
#include <unordered_set>


namespace sma
{

Message::Message(Builder&& builder, Address sender)
  : type_(builder.type)
  , sender_(sender)
  , recipients_(std::move(builder.recipients))
  , body_data(builder.body)
  , body_len(builder.len)
{
  assert(recipients_.size() <= field_nr_recipients_max);
  assert(body_len <= field_body_len_max);
}

Message::Message(Type type,
                 Address sender,
                 std::unordered_set<Address>&& recipients,
                 const std::uint8_t* body,
                 std::size_t len)
  : type_(type)
  , sender_(sender)
  , recipients_(recipients)
  , body_data(body)
  , body_len(len)
{
  assert(recipients_.size() <= field_nr_recipients_max);
  assert(len <= field_body_len_max);
}

Message::Message(const std::uint8_t* src, std::size_t len)
{
  len -= static_header_size();
  assert(len >= 0);

  src = read(src, sender_);

  field_nr_recipients_type nr_recipients{0};
  src = read(src, nr_recipients);

  if (nr_recipients > 0) {
    len -= nr_recipients * sizeof(Address);
    assert(len >= 0);

    std::unordered_set<Address>::size_type n{nr_recipients};
    recipients_ = std::unordered_set<Address>(n);
    while (n-- > 0) {
      Address r;
      src = read(src, r);
      recipients_.insert(std::move(r));
    }
  }

  src = read(src, type_);
  src = read(src, body_len);

  len -= body_len;
  assert(len >= 0);

  // don't copy
  body_data = src;
}

std::uint8_t* Message::put_in(std::uint8_t* dst, std::size_t len) const
{
  assert(len >= serialized_size());

  dst = write(sender_, dst);
  dst = write(std::uint8_t(recipients_.size()), dst);
  for (auto& recipient : recipients_)
    dst = write(recipient, dst);
  dst = write(type_, dst);
  dst = write(body_len, dst);

  len -= header_size();

  for (field_body_len_type i = 0; i < body_len && len-- > 0; ++i)
    *dst++ = body_data[i];

  return dst;
}

std::uint8_t* Message::write(const std::uint8_t& in, std::uint8_t* dst)
{
  *dst++ = in;
  return dst;
}
std::uint8_t* Message::write(const std::uint16_t& in, std::uint8_t* dst)
{
  *dst++ = in >> 8 & 0xFF;
  *dst++ = in & 0xFF;
  return dst;
}
std::uint8_t* Message::write(const std::uint32_t& in, std::uint8_t* dst)
{
  *dst++ = in >> 24 & 0xFF;
  *dst++ = in >> 16 & 0xFF;
  *dst++ = in >> 8 & 0xFF;
  *dst++ = in & 0xFF;
  return dst;
}
std::uint8_t* Message::write(const std::uint64_t& in, std::uint8_t* dst)
{
  *dst++ = in >> 56 & 0xFF;
  *dst++ = in >> 48 & 0xFF;
  *dst++ = in >> 40 & 0xFF;
  *dst++ = in >> 32 & 0xFF;
  *dst++ = in >> 24 & 0xFF;
  *dst++ = in >> 16 & 0xFF;
  *dst++ = in >> 8 & 0xFF;
  *dst++ = in & 0xFF;
  return dst;
}


const std::uint8_t* Message::read(const std::uint8_t* src, std::uint8_t& out)
{
  out = *src++;
  return src;
}
const std::uint8_t* Message::read(const std::uint8_t* src, std::uint16_t& out)
{
  out = std::uint16_t{*src++} << 8;
  out |= *src++;
  return src;
}
const std::uint8_t* Message::read(const std::uint8_t* src, std::uint32_t& out)
{
  out = std::uint32_t{*src++} << 24;
  out |= std::uint32_t{*src++} << 16;
  out |= std::uint32_t{*src++} << 8;
  out |= *src++;
  return src;
}
const std::uint8_t* Message::read(const std::uint8_t* src, std::uint64_t& out)
{
  out = std::uint64_t{*src++} << 56;
  out |= std::uint64_t{*src++} << 48;
  out |= std::uint64_t{*src++} << 40;
  out |= std::uint64_t{*src++} << 32;
  out |= std::uint64_t{*src++} << 24;
  out |= std::uint64_t{*src++} << 16;
  out |= std::uint64_t{*src++} << 8;
  out |= *src++;
  return src;
}

bool Message::operator==(const Message& other) const
{
  if (!std::equal_to<Address>()(sender_, other.sender_))
    return false;

  if (recipients_.size() != other.recipients_.size())
    return false;

  std::unordered_set<Address> lhs(recipients_.cbegin(), recipients_.cend());
  for (auto& recipient : other.recipients_)
    if (lhs.find(recipient) == lhs.end())
      return false;

  std::size_t l(-1);
  while (++l < body_len && body_data[l] == other.body_data[l])
    ;
  return l == body_len;
}
}
