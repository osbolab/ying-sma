#include "bytebuf.hh"
#include "bytes.hh"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <utility>


namespace sma
{


/******************************************************************************
 * bytebuf - Static Factories
 */
bytebuf bytebuf::allocate(std::size_t len) { return bytebuf(len); }
bytebuf bytebuf::wrap(void* buf, std::size_t len)
{
  return bytebuf(static_cast<std::uint8_t*>(buf), len);
}
bytebuf bytebuf::copy(const void* buf, std::size_t len)
{
  return bytebuf(static_cast<const std::uint8_t*>(buf), len);
}
/* bytebuf- Static Factories
 *****************************************************************************/


/******************************************************************************
 * bytebuf - C/dtor / allocator
 */
bytebuf::bytebuf(bytebuf&& rhs)
  : bytebuf::view(std::move(rhs.b), rhs.lim, rhs.pos)
  , owner(rhs.owner)
  , cap(rhs.cap)
{
  rhs.b = nullptr;
  rhs.owner = false;
  rhs.cap = rhs.lim = rhs.pos = 0;
}
bytebuf::bytebuf(const bytebuf& rhs)
  : bytebuf::view(new std::uint8_t[rhs.cap], rhs.lim, rhs.pos)
  , cap(rhs.cap)
  , owner(true)
{
  std::memcpy(b, rhs.b, cap);
}
bytebuf::bytebuf(std::size_t capacity)
  : bytebuf::view(new std::uint8_t[capacity], capacity)
  , cap(capacity)
  , owner(true)
{
}
bytebuf::bytebuf(const std::uint8_t* src, std::size_t len)
  : bytebuf::view(new std::uint8_t[len], len)
  , cap(len)
  , owner(true)
{
  std::memcpy(b, src, len);
}
bytebuf::bytebuf(std::uint8_t* buf, std::size_t len)
  : bytebuf::view(buf, len)
  , cap(len)
  , owner(false)
{
}
bytebuf::bytebuf(std::uint8_t* buf,
                 std::size_t cap,
                 std::size_t lim,
                 std::size_t pos)
  : bytebuf::view(buf, lim, pos)
  , cap(cap)
  , owner(false)
{
}

// Allocate a new buffer equal in len to this and copy this buffer's full
// contents to it.
bytebuf bytebuf::duplicate() { return bytebuf(*this); }

bytebuf::~bytebuf()
{
  if (owner)
    delete[] b;
}
/* bytebuf- C/dtor / allocator
 *****************************************************************************/

/******************************************************************************
 * bytebuf - Operators
 */
bytebuf& bytebuf::operator=(bytebuf&& rhs)
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
bytebuf& bytebuf::operator=(const bytebuf& rhs)
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
std::uint8_t& bytebuf::operator[](std::size_t i)
{
  assert(i < lim);
  return b[i];
}
std::uint8_t& bytebuf::operator*()
{
  assert(pos < lim);
  return b[pos];
}
/* bytebuf- Operators
 *****************************************************************************/

/******************************************************************************
 * bytebuf - Navigation
 */
bytebuf& bytebuf::seek(std::size_t i)
{
  bytebuf::view::seek(i);
  return *this;
}
bytebuf& bytebuf::limit(std::size_t newlim)
{
  assert(newlim <= cap);
  lim = newlim;
  if (pos > lim)
    pos = lim;
  return *this;
}
bytebuf& bytebuf::flip()
{
  lim = pos;
  pos = 0;
  return *this;
}
bytebuf& bytebuf::clear()
{
  lim = cap;
  pos = 0;
  return *this;
}
/* bytebuf - Navigation
 *****************************************************************************/

/******************************************************************************
 * bytebuf  - Covariant overrides on read
 */
template <>
bytebuf& bytebuf::operator>>(const arrcopy_w<std::uint8_t>& dst)
{
  bytebuf::view::operator>>(dst);
  return *this;
}
template <>
bytebuf& bytebuf::operator>>(const arrcopy_w<char>& dst)
{
  bytebuf::view::operator>>(dst);
  return *this;
}
template <>
bytebuf& bytebuf::operator>>(std::uint8_t& v)
{
  bytebuf::view::operator>>(v);
  return *this;
}
template <>
bytebuf& bytebuf::operator>>(std::uint16_t& v)
{
  v = get<std::uint16_t>();
  return *this;
}
template <>
bytebuf& bytebuf::operator>>(std::uint32_t& v)
{
  v = get<std::uint32_t>();
  return *this;
}
template <>
bytebuf& bytebuf::operator>>(std::uint64_t& v)
{
  v = get<std::uint64_t>();
  return *this;
}
/* bytebuf - Covariant overrides on read
 *****************************************************************************/


/******************************************************************************
 * bytebuf - Writing
 */
bytebuf& bytebuf::put(const void* src, std::size_t len)
{
  assert(remaining() >= len);
  std::memcpy(b + pos, src, len);
  pos += len;
  return *this;
}

bytebuf& bytebuf::replace(const void* src, std::size_t len)
{
  assert(len >= cap);
  std::memcpy(b, src, cap);
  clear();
  return *this;
}

template <>
bytebuf& bytebuf::put(const std::uint8_t& v)
{
  assert(remaining() >= 1);
  b[pos++] = v;
  return *this;
}
template <>
bytebuf& bytebuf::put(const std::uint16_t& v)
{
  assert(remaining() >= 2);
  b[pos++] = v >> 8;
  b[pos++] = v & 0xFF;
  return *this;
}
template <>
bytebuf& bytebuf::put(const std::uint32_t& v)
{
  assert(remaining() >= 4);
  b[pos++] = v >> 24;
  b[pos++] = (v >> 16) & 0xFF;
  b[pos++] = (v >> 8) & 0xFF;
  b[pos++] = v & 0xFF;
  return *this;
}
template <>
bytebuf& bytebuf::put(const std::uint64_t& v)
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
/* bytebuf - Writing
 *****************************************************************************/
}
