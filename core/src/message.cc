#include "message.hh"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
#include <unordered_set>


namespace sma
{

Message::Message(Type type,
                 Recipient sender,
                 std::vector<Recipient> recipients,
                 const uint8_t* body,
                 size_t len)
  : type(type)
  , sender(sender)
  , recipients(std::move(recipients))
  , body_data(body)
  , body_len(len)
{
  assert(recipients.size() <= field_nr_recipients_max);
  assert(len <= field_body_len_max);
}

Message::Message(Message&& m)
  : type(m.type)
  , sender(m.sender)
  , body_data(m.body_data)
  , body_len(m.body_len)
{
  m.type = 0;
  m.sender = Recipient{{0}};
  std::swap(recipients, m.recipients);
  m.body_data = nullptr;
  m.body_len = 0;
}

Message::Message(const uint8_t* src, size_t len)
{
  src = Message::read(src, sender);

  field_nr_recipients_type nr_recipients{0};
  src = Message::read(src, nr_recipients);

  if (nr_recipients > 0) {
    const std::vector<Recipient>::size_type nr{nr_recipients};
    recipients = std::vector<Recipient>(nr);
    for (std::vector<Recipient>::size_type i = 0; i < nr; ++i) {
      src = Message::read(src, recipients[i]);
    }
  }

  src = Message::read(src, type);
  src = Message::read(src, body_len);

  // don't copy
  body_data = src;
}

uint8_t* Message::serialize_to(uint8_t* dst, size_t len) const
{
  assert(len >= serialized_size());

  dst = write(sender, dst);
  dst = write(uint8_t(recipients.size()), dst);
  for (auto& recipient : recipients)
    dst = write(recipient, dst);
  dst = write(type, dst);
  dst = write(body_len, dst);

  len -= header_size();

  for (field_body_len_type i = 0; i < body_len && len-- > 0; ++i)
    *dst++ = body_data[i];

  return dst;
}

uint8_t* Message::write(const Recipient& in, uint8_t* dst)
{
  for (size_t i = 0; i < sizeof(Recipient::uint8); ++i)
    *dst++ = in.uint8[i];
  return dst;
}
uint8_t* Message::write(const uint8_t& in, uint8_t* dst)
{
  *dst++ = in;
  return dst;
}
uint8_t* Message::write(const uint16_t& in, uint8_t* dst)
{
  *dst++ = in >> 8 & 0xFF;
  *dst++ = in & 0xFF;
  return dst;
}
uint8_t* Message::write(const uint32_t& in, uint8_t* dst)
{
  *dst++ = in >> 24 & 0xFF;
  *dst++ = in >> 16 & 0xFF;
  *dst++ = in >> 8 & 0xFF;
  *dst++ = in & 0xFF;
  return dst;
}
uint8_t* Message::write(const uint64_t& in, uint8_t* dst)
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


const uint8_t* Message::read(const uint8_t* src, Recipient& out)
{
  for (size_t i = 0; i < sizeof(Recipient::uint8); ++i)
    out.uint8[i] = *src++;
  return src;
}
const uint8_t* Message::read(const uint8_t* src, uint8_t& out)
{
  out = *src++;
  return src;
}
const uint8_t* Message::read(const uint8_t* src, uint16_t& out)
{
  out |= uint16_t{*src++} << 8;
  out |= *src++;
  return src;
}
const uint8_t* Message::read(const uint8_t* src, uint32_t& out)
{
  out |= uint32_t{*src++} << 24;
  out |= uint32_t{*src++} << 16;
  out |= uint32_t{*src++} << 8;
  out |= *src++;
  return src;
}
const uint8_t* Message::read(const uint8_t* src, uint64_t& out)
{
  out = uint64_t{*src++} << 56;
  out |= uint64_t{*src++} << 48;
  out |= uint64_t{*src++} << 40;
  out |= uint64_t{*src++} << 32;
  out |= uint64_t{*src++} << 24;
  out |= uint64_t{*src++} << 16;
  out |= uint64_t{*src++} << 8;
  out |= *src++;
  return src;
}

Message& Message::operator=(Message&& m)
{
  std::swap(type, m.type);
  std::swap(sender, m.sender);
  std::swap(recipients, m.recipients);
  std::swap(body_data, m.body_data);
  std::swap(body_len, m.body_len);
  return *this;
}

bool Message::operator==(const Message& other) const
{
  if (!std::equal_to<Recipient>()(sender, other.sender))
    return false;

  if (recipients.size() != other.recipients.size())
    return false;

  std::unordered_set<Recipient> lhs(recipients.cbegin(), recipients.cend());
  for (auto& recipient : other.recipients)
    if (lhs.find(recipient) == lhs.end())
      return false;

  size_t l(-1);
  while (++l < body_len && body_data[l] == other.body_data[l])
    ;
  return l == body_len;
}

size_t Message::header_size() const
{
  return sizeof type + sizeof(field_nr_recipients_type)
         + sizeof(Recipient::uint8) * (recipients.size() + 1)
         + sizeof(field_body_len_type) + sizeof body_len;
}
}
