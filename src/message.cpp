#include <sma/Message.hpp>
#include <sma/Buffer.hpp>

#include <cstdint>
#include <cassert>
#include <utility>
#include <vector>
#include <iostream>


namespace sma
{

// clang-format off
std::ostream& operator<<(std::ostream& os, const Message& msg)
{
  os << "Message {\n   type: " << std::size_t{msg.t}
     <<          "\n , data: (" << msg.szdata << " bytes)\n}";
  return os;
}
// clang-format on


Buffer& operator<<(Buffer& dst, Message::Header const& src)
{
  return dst << src.type << src.data_size;
}
Buffer::View& operator>>(Buffer::View& src, Message::Header& dst)
{
  src >> dst.type;
  src >> dst.data_size;
}

Message Message::wrap_from(std::uint8_t const* src, std::size_t size)
{
  auto view = Buffer::View(src, size);
  Header header;
  view >> header;
  return Message(header, view.cbuf());
}

Message Message::copy_from(std::uint8_t const* src, std::size_t size)
{
  auto view = Buffer::View(src, size);
  Header header;
  view >> header;
  auto data = std::make_unique<std::uint8_t[]>(header.data_size);
  view.get(data.get(), header.data_size);
  return Message(header, std::move(data));
}

std::size_t Message::serialize_to(Buffer& dst) const
{
  assert(dst.remaining() >= serialized_size());
  std::size_t start = dst.position();
  dst << t;
  dst << static_cast<field_size_type>(szdata);
  dst.put(data, szdata);
  return dst.position() - start;
}
std::size_t Message::serialize_to(std::uint8_t* dst, std::size_t szdata) const
{
  auto buf = Buffer::wrap(dst, szdata);
  return serialize_to(buf);
}
std::size_t Message::serialized_size() const
{
  // clang-format off
  return sizeof(Message_type)
         + sizeof(field_size_type)
         + szdata;
  // clang-format on
}

Message::Message(Message_type type,
                 std::uint8_t const* data,
                 std::size_t szdata)
  : t(type)
  , data(data)
  , szdata(szdata)
{
}
Message::Message(Message&& r)
  : t(std::move(r.t))
  , data(r.data)
  , szdata(r.szdata)
{
  r.data = nullptr;
  r.szdata = 0;
}
Message::Message(const Message& r)
  : t(r.t)
  , data(r.data)
  , szdata(r.szdata)
{
}

Message& Message::operator=(Message&& r)
{
  std::swap(t, r.t);
  std::swap(data, r.data);
  std::swap(szdata, r.szdata);
  return *this;
}
Message& Message::operator=(const Message& r)
{
  t = r.t;
  data = r.data;
  szdata = r.szdata;
  return *this;
}
}
