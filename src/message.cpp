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

Message Message::wrap(Type type,
                      Weight weight,
                      std::uint8_t const* data,
                      data_size_type size)
{
  return Message(
}


Message Message::copy(Type type,
                      Weight weight,
                      std::uint8_t const* data,
                      data_size_type size)
{
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
