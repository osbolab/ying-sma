#include "message.hh"

#include <cassert>
#include <utility>
#include <vector>


namespace sma
{

Message::Message(Type type,
                 Address sender,
                 std::vector<Address> recipients,
                 const std::uint8_t* body,
                 std::size_t len)
  : type(type)
  , sender(sender)
  , recipients(recipients)
  , body_data(body)
  , body_len(len)
{
}

Message::Message(Message&& m)
  : type(m.type)
  , sender(m.sender)
  , body_data(m.body_data)
  , body_len(m.body_len)
{
  m.type = 0;
  m.sender = Address {{0}};
  std::swap(recipients, m.recipients);
  m.body_data = nullptr;
  m.body_len = 0;
}

Message::Message(const std::uint8_t* src, std::size_t len)
{
  std::size_t i;

  // sender
  for (i = 0; i < sizeof(Address::data); ++i)
    sender.data[i] = *src++;

  // nr_recipients
  std::size_t nr_recipients{0};
  for (i = 0; i < sizeof nr_recipients; ++i)
    nr_recipients |= (*src++ << (i << 3));

  // recipient[]
  for (std::size_t recp = 0; recp < nr_recipients; ++recp)
  {
    Address recipient;
    for (i = 0; i < sizeof(Address::data); ++i)
      recipient.data[i] = *src++;
    recipients.push_back(recipient);
  }

  // message type
  for (i = 0; i < sizeof type; ++i)
    type |= (*src++ << (i << 3));

  // body len
  for (i = 0; i < sizeof body_len; ++i)
    body_len |= (*src++ << (i << 3));

  // body (don't copy)
  body_data = src;
}

std::uint8_t* Message::serialize_to(std::uint8_t* dst, std::size_t len) const
{
  assert(len >= serialized_size());

  std::size_t i;

  // sender
  for (i = 0; i < sizeof(Address::data); ++i)
    *dst++ = sender.data[i];

  // nr_recipients
  const std::size_t nr_recipients = recipients.size();
  for (i = 0; i < sizeof nr_recipients; ++i)
    *dst++ = (nr_recipients >> (i << 3)) & 0xFF;

  // recipient[]
  for (auto& recipient : recipients)
    for (i = 0; i < sizeof(Address::data); ++i)
      *dst++ = recipient.data[i];

  // message type
  for (i = 0; i < sizeof type; ++i)
    *dst++ = (type >> (i << 3)) & 0xFF;

  // body len
  for (i = 0; i < sizeof body_len; ++i)
    *dst++ = (body_len >> (i << 3)) & 0xFF;

  len -= header_size();

  // body
  for (i = 0; i < body_len && len-- > 0; ++i)
    *dst++ = body_data[i];

  return dst;
}

Message& Message::operator=(Message&& m)
{
  std::swap(type, m.type);
  std::swap(sender, m.sender);
  std::swap(recipients, m.recipients);
  std::swap(body_data, m.body_data);
  std::swap(body_len, m.body_len);
}

bool Message::operator==(const Message& other) const
{
  std::size_t l(-1);
  while (++l < body_len && body_data[l] == other.body_data[l])
    ;
  return l == body_len;
}
bool Message::operator!=(const Message& other) const
{
  return !(*this == other);
}

std::size_t Message::serialized_size() const
{
  return header_size() + body_len;
}

std::size_t Message::header_size() const
{
  return sizeof type
    + sizeof(Address) * (recipients.size() + 1)
    + sizeof(std::size_t)
    + sizeof body_len;
}

std::size_t Message::body_size() const
{
  return body_len;
}

const std::uint8_t* Message::body() const
{
  return body_data;
}

}
