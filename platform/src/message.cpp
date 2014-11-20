#include <sma/message.hpp>
#include <sma/buffer.hpp>

#include <cstdint>
#include <cassert>
#include <utility>
#include <vector>
#include <iostream>


namespace sma
{

// clang-format off
std::ostream& operator<<(std::ostream& os, const message& msg)
{
  os << "message {\n   type: " << std::size_t{msg.t}
      <<         "\n , data: (" << msg.szdata << " bytes)\n}";
  return os;
}
// clang-format on

/******************************************************************************
 * message - Deserialization/Construction
 */
message::message(buffer::view src)
{
  src >> t;

  field_size_type szdata;
  src >> szdata;
  szdata = static_cast<std::size_t>(szdata);

  data = src.cbuf();
}
message::message(std::uint8_t const* src, std::size_t szdata)
  : message(buffer::view::of(src, szdata))
{
}
/* message - Deserialization/Construction
******************************************************************************/

/******************************************************************************
 * message - Serialization
 */
std::size_t message::serialize_to(buffer& dst) const
{
  assert(dst.remaining() >= serialized_size());
  std::size_t start = dst.position();
  dst << t;
  dst << static_cast<field_size_type>(szdata);
  dst.put(data, szdata);
  return dst.position() - start;
}
std::size_t message::serialize_to(std::uint8_t* dst, std::size_t szdata) const
{
  auto buf = buffer::wrap(dst, szdata);
  return serialize_to(buf);
}
std::size_t message::serialized_size() const
{
  // clang-format off
  return sizeof(message_type)
         + sizeof(field_size_type)
         + szdata;
  // clang-format on
}
/* message - Serialization
******************************************************************************/

/******************************************************************************
 * message - Regular Constructon
 */
message::message(message_type type, std::uint8_t const* data, std::size_t szdata)
  : t(type)
  , data(data)
  , szdata(szdata)
{
}
message::message(message&& r)
  : t(std::move(r.t))
  , data(r.data)
  , szdata(r.szdata)
{
  r.data = nullptr;
  r.szdata = 0;
}
message::message(const message& r)
  : t(r.t)
  , data(r.data)
  , szdata(r.szdata)
{
}

message& message::operator=(message&& r)
{
  std::swap(t, r.t);
  std::swap(data, r.data);
  std::swap(szdata, r.szdata);
  return *this;
}
message& message::operator=(const message& r)
{
  t = r.t;
  data = r.data;
  szdata = r.szdata;
  return *this;
}
/* message - Regular Construction
 *****************************************************************************/
}
