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
buffer buffer::copy(void const* buf, std::size_t len)
{
  return buffer(static_cast<std::uint8_t const*>(buf), len);
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
buffer::buffer(buffer const& rhs)
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
buffer::buffer(std::uint8_t const* src, std::size_t len)
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
buffer& buffer::operator=(buffer const& rhs)
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
buffer& buffer::operator>>(std::uint8_t& dst)
{
  dst = get_uint8();
  return *this;
}
buffer& buffer::operator>>(std::uint16_t& dst)
{
  dst = get_uint16();
  return *this;
}
buffer& buffer::operator>>(std::uint32_t& dst)
{
  dst = get_uint32();
  return *this;
}
buffer& buffer::operator>>(std::uint64_t& dst)
{
  dst = get_uint64();
  return *this;
}
/* buffer - Covariant overrides on read
 *****************************************************************************/


/******************************************************************************
 * buffer - Writing
 */
buffer& buffer::replace(void const* src, std::size_t len)
{
  assert(len >= cap);
  std::memcpy(b, src, cap);
  clear();
  return *this;
}
buffer& buffer::put(void const* src, std::size_t len)
{
  assert(remaining() >= len);
  std::memcpy(b + pos, src, len);
  pos += len;
  return *this;
}
buffer& buffer::put_8(std::uint8_t const& src)
{
  assert(remaining() >= 1);
  b[pos++] = src;
  return *this;
}
buffer& buffer::put_16(std::uint16_t const& src)
{
  assert(remaining() >= 2);
  b[pos++] = src >> 8;
  b[pos++] = src & 0xFF;
  return *this;
}
buffer& buffer::put_32(std::uint32_t const& src)
{
  assert(remaining() >= 4);
  b[pos++] = src >> 24;
  b[pos++] = (src >> 16) & 0xFF;
  b[pos++] = (src >> 8) & 0xFF;
  b[pos++] = src & 0xFF;
  return *this;
}
buffer& buffer::put_64(std::uint64_t const& src)
{
  assert(remaining() >= 8);
  b[pos++] = src >> 56;
  b[pos++] = (src >> 48) & 0xFF;
  b[pos++] = (src >> 40) & 0xFF;
  b[pos++] = (src >> 32) & 0xFF;
  b[pos++] = (src >> 24) & 0xFF;
  b[pos++] = (src >> 16) & 0xFF;
  b[pos++] = (src >> 8) & 0xFF;
  b[pos++] = src & 0xFF;
  return *this;
}

buffer& buffer::operator<<(std::uint8_t const& src)
{
  put_8(src);
  return *this;
}
buffer& buffer::operator<<(std::uint16_t const& src)
{
  put_16(src);
  return *this;
}
buffer& buffer::operator<<(std::uint32_t const& src)
{
  put_32(src);
  return *this;
}
buffer& buffer::operator<<(std::uint64_t const& src)
{
  put_64(src);
  return *this;
}
/* buffer - Writing
 *****************************************************************************/
}
