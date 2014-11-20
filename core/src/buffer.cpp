#include <sma/buffer.hpp>
#include <sma/bytes.hpp>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <utility>


namespace sma
{


/******************************************************************************
 * buffer - Static Factories
 */
buffer buffer::allocate(std::size_t len) { return buffer(len); }
buffer buffer::wrap(void* buf, std::size_t len)
{
  return buffer(static_cast<std::uint8_t*>(buf), len);
}
buffer buffer::copy(const void* buf, std::size_t len)
{
  return buffer(static_cast<const std::uint8_t*>(buf), len);
}
/* buffer- Static Factories
 *****************************************************************************/


/******************************************************************************
 * buffer - C/dtor / allocator
 */
buffer::buffer(buffer&& rhs)
  : buffer::view(std::move(rhs.b), rhs.lim, rhs.pos)
  , owner(rhs.owner)
  , cap(rhs.cap)
{
  rhs.b = nullptr;
  rhs.owner = false;
  rhs.cap = rhs.lim = rhs.pos = 0;
}
buffer::buffer(const buffer& rhs)
  : buffer::view(new std::uint8_t[rhs.cap], rhs.lim, rhs.pos)
  , cap(rhs.cap)
  , owner(true)
{
  std::memcpy(b, rhs.b, cap);
}
buffer::buffer(std::size_t capacity)
  : buffer::view(new std::uint8_t[capacity], capacity)
  , cap(capacity)
  , owner(true)
{
}
buffer::buffer(const std::uint8_t* src, std::size_t len)
  : buffer::view(new std::uint8_t[len], len)
  , cap(len)
  , owner(true)
{
  std::memcpy(b, src, len);
}
buffer::buffer(std::uint8_t* buf, std::size_t len)
  : buffer::view(buf, len)
  , cap(len)
  , owner(false)
{
}
buffer::buffer(std::uint8_t* buf,
                 std::size_t cap,
                 std::size_t lim,
                 std::size_t pos)
  : buffer::view(buf, lim, pos)
  , cap(cap)
  , owner(false)
{
}

// Allocate a new buffer equal in len to this and copy this buffer's full
// contents to it.
buffer buffer::duplicate() { return buffer(*this); }

buffer::~buffer()
{
  if (owner)
    delete[] b;
}
/* buffer- C/dtor / allocator
 *****************************************************************************/

/******************************************************************************
 * buffer - Operators
 */
buffer& buffer::operator=(buffer&& rhs)
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
buffer& buffer::operator=(const buffer& rhs)
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
std::uint8_t& buffer::operator[](std::size_t i)
{
  assert(i < lim);
  return b[i];
}
std::uint8_t& buffer::operator*()
{
  assert(pos < lim);
  return b[pos];
}
/* buffer- Operators
 *****************************************************************************/

/******************************************************************************
 * buffer - Navigation
 */
buffer& buffer::seek(std::size_t i)
{
  buffer::view::seek(i);
  return *this;
}
buffer& buffer::limit(std::size_t newlim)
{
  assert(newlim <= cap);
  lim = newlim;
  if (pos > lim)
    pos = lim;
  return *this;
}
buffer& buffer::flip()
{
  lim = pos;
  pos = 0;
  return *this;
}
buffer& buffer::clear()
{
  lim = cap;
  pos = 0;
  return *this;
}
/* buffer - Navigation
 *****************************************************************************/

/******************************************************************************
 * buffer  - Covariant overrides on read
 */
template <>
buffer& buffer::operator>>(const arrcopy_w<std::uint8_t>& dst)
{
  buffer::view::operator>>(dst);
  return *this;
}
template <>
buffer& buffer::operator>>(const arrcopy_w<char>& dst)
{
  buffer::view::operator>>(dst);
  return *this;
}
template <>
buffer& buffer::operator>>(std::uint8_t& v)
{
  buffer::view::operator>>(v);
  return *this;
}
template <>
buffer& buffer::operator>>(std::uint16_t& v)
{
  v = get<std::uint16_t>();
  return *this;
}
template <>
buffer& buffer::operator>>(std::uint32_t& v)
{
  v = get<std::uint32_t>();
  return *this;
}
template <>
buffer& buffer::operator>>(std::uint64_t& v)
{
  v = get<std::uint64_t>();
  return *this;
}
/* buffer - Covariant overrides on read
 *****************************************************************************/


/******************************************************************************
 * buffer - Writing
 */
buffer& buffer::put(const void* src, std::size_t len)
{
  assert(remaining() >= len);
  std::memcpy(b + pos, src, len);
  pos += len;
  return *this;
}

buffer& buffer::replace(const void* src, std::size_t len)
{
  assert(len >= cap);
  std::memcpy(b, src, cap);
  clear();
  return *this;
}

template <>
buffer& buffer::put(const std::uint8_t& v)
{
  assert(remaining() >= 1);
  b[pos++] = v;
  return *this;
}
template <>
buffer& buffer::put(const std::uint16_t& v)
{
  assert(remaining() >= 2);
  b[pos++] = v >> 8;
  b[pos++] = v & 0xFF;
  return *this;
}
template <>
buffer& buffer::put(const std::uint32_t& v)
{
  assert(remaining() >= 4);
  b[pos++] = v >> 24;
  b[pos++] = (v >> 16) & 0xFF;
  b[pos++] = (v >> 8) & 0xFF;
  b[pos++] = v & 0xFF;
  return *this;
}
template <>
buffer& buffer::put(const std::uint64_t& v)
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
/* buffer - Writing
 *****************************************************************************/
}
