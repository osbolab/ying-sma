#include <sma/core/byte_buf.hpp>
#include <sma/core/bytes.hpp>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <utility>


namespace sma
{


/******************************************************************************
 * byte_buf - Static Factories
 */
byte_buf byte_buf::allocate(std::size_t len) { return byte_buf(len); }
byte_buf byte_buf::wrap(void* buf, std::size_t len)
{
  return byte_buf(static_cast<std::uint8_t*>(buf), len);
}
byte_buf byte_buf::copy(const void* buf, std::size_t len)
{
  return byte_buf(static_cast<const std::uint8_t*>(buf), len);
}
/* byte_buf- Static Factories
 *****************************************************************************/


/******************************************************************************
 * byte_buf - C/dtor / allocator
 */
byte_buf::byte_buf(byte_buf&& rhs)
  : byte_buf::view(std::move(rhs.b), rhs.lim, rhs.pos)
  , owner(rhs.owner)
  , cap(rhs.cap)
{
  rhs.b = nullptr;
  rhs.owner = false;
  rhs.cap = rhs.lim = rhs.pos = 0;
}
byte_buf::byte_buf(const byte_buf& rhs)
  : byte_buf::view(new std::uint8_t[rhs.cap], rhs.lim, rhs.pos)
  , cap(rhs.cap)
  , owner(true)
{
  std::memcpy(b, rhs.b, cap);
}
byte_buf::byte_buf(std::size_t capacity)
  : byte_buf::view(new std::uint8_t[capacity], capacity)
  , cap(capacity)
  , owner(true)
{
}
byte_buf::byte_buf(const std::uint8_t* src, std::size_t len)
  : byte_buf::view(new std::uint8_t[len], len)
  , cap(len)
  , owner(true)
{
  std::memcpy(b, src, len);
}
byte_buf::byte_buf(std::uint8_t* buf, std::size_t len)
  : byte_buf::view(buf, len)
  , cap(len)
  , owner(false)
{
}
byte_buf::byte_buf(std::uint8_t* buf,
                 std::size_t cap,
                 std::size_t lim,
                 std::size_t pos)
  : byte_buf::view(buf, lim, pos)
  , cap(cap)
  , owner(false)
{
}

// Allocate a new buffer equal in len to this and copy this buffer's full
// contents to it.
byte_buf byte_buf::duplicate() { return byte_buf(*this); }

byte_buf::~byte_buf()
{
  if (owner)
    delete[] b;
}
/* byte_buf- C/dtor / allocator
 *****************************************************************************/

/******************************************************************************
 * byte_buf - Operators
 */
byte_buf& byte_buf::operator=(byte_buf&& rhs)
{
  if (owner)
    delete[] b;
  b = rhs.b;
  owner = rhs.owner;
  cap = rhs.cap;
  lim = rhs.lim;
  pos = rhs.pos;
  rhs.b = nullptr;
  rhs.owner = false;
  rhs.cap = rhs.lim = rhs.pos = 0;
  return *this;
}
byte_buf& byte_buf::operator=(const byte_buf& rhs)
{
  if (owner)
    delete[] b;
  owner = true;
  b = new std::uint8_t[rhs.cap];
  cap = rhs.cap;
  lim = rhs.lim;
  pos = rhs.pos;
  std::memcpy(b, rhs.b, cap);
  return *this;
}
std::uint8_t& byte_buf::operator[](std::size_t i)
{
  assert(i < lim);
  return b[i];
}
std::uint8_t& byte_buf::operator*()
{
  assert(pos < lim);
  return b[pos];
}
/* byte_buf- Operators
 *****************************************************************************/

/******************************************************************************
 * byte_buf - Navigation
 */
byte_buf& byte_buf::seek(std::size_t i)
{
  byte_buf::view::seek(i);
  return *this;
}
byte_buf& byte_buf::limit(std::size_t newlim)
{
  assert(newlim <= cap);
  lim = newlim;
  if (pos > lim)
    pos = lim;
  return *this;
}
byte_buf& byte_buf::flip()
{
  lim = pos;
  pos = 0;
  return *this;
}
byte_buf& byte_buf::clear()
{
  lim = cap;
  pos = 0;
  return *this;
}
/* byte_buf - Navigation
 *****************************************************************************/

/******************************************************************************
 * byte_buf  - Covariant overrides on read
 */
template <>
byte_buf& byte_buf::operator>>(const arrcopy_w<std::uint8_t>& dst)
{
  byte_buf::view::operator>>(dst);
  return *this;
}
template <>
byte_buf& byte_buf::operator>>(const arrcopy_w<char>& dst)
{
  byte_buf::view::operator>>(dst);
  return *this;
}
template <>
byte_buf& byte_buf::operator>>(std::uint8_t& v)
{
  byte_buf::view::operator>>(v);
  return *this;
}
template <>
byte_buf& byte_buf::operator>>(std::uint16_t& v)
{
  v = get<std::uint16_t>();
  return *this;
}
template <>
byte_buf& byte_buf::operator>>(std::uint32_t& v)
{
  v = get<std::uint32_t>();
  return *this;
}
template <>
byte_buf& byte_buf::operator>>(std::uint64_t& v)
{
  v = get<std::uint64_t>();
  return *this;
}
/* byte_buf - Covariant overrides on read
 *****************************************************************************/


/******************************************************************************
 * byte_buf - Writing
 */
byte_buf& byte_buf::put(const void* src, std::size_t len)
{
  assert(remaining() >= len);
  std::memcpy(b + pos, src, len);
  pos += len;
  return *this;
}

byte_buf& byte_buf::replace(const void* src, std::size_t len)
{
  assert(len >= cap);
  std::memcpy(b, src, cap);
  clear();
  return *this;
}

template <>
byte_buf& byte_buf::put(const std::uint8_t& v)
{
  assert(remaining() >= 1);
  b[pos++] = v;
  return *this;
}
template <>
byte_buf& byte_buf::put(const std::uint16_t& v)
{
  assert(remaining() >= 2);
  b[pos++] = v >> 8;
  b[pos++] = v & 0xFF;
  return *this;
}
template <>
byte_buf& byte_buf::put(const std::uint32_t& v)
{
  assert(remaining() >= 4);
  b[pos++] = v >> 24;
  b[pos++] = (v >> 16) & 0xFF;
  b[pos++] = (v >> 8) & 0xFF;
  b[pos++] = v & 0xFF;
  return *this;
}
template <>
byte_buf& byte_buf::put(const std::uint64_t& v)
{
  assert(remaining() >= 8);
  b[pos++] = v >> 56;
  b[pos++] = (v >> 48) & 0xFF;
  b[pos++] = (v >> 40) & 0xFF;
  b[pos++] = (v >> 32) & 0xFF;
  b[pos++] = (v >> 24) & 0xFF;
  b[pos++] = (v >> 16) & 0xFF;
  b[pos++] = (v >> 8) & 0xFF;
  b[pos++] = v & 0xFF;
  return *this;
}
/* byte_buf - Writing
 *****************************************************************************/
}
