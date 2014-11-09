#include "bytebuffer.hh"
#include "bytes.hh"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <utility>


namespace sma
{

/******************************************************************************
 * ByteView - Static Factories
 */
ByteView ByteView::wrap(const std::uint8_t* src, std::size_t len)
{
  return ByteView(const_cast<std::uint8_t*>(src), len, 0);
}
/* ByteView - Static Factories
 *****************************************************************************/

/******************************************************************************
 * ByteView - C/dtor
 */

ByteView::ByteView()
  : b(nullptr)
  , lim(0)
  , pos(0)
{
}
ByteView::ByteView(std::uint8_t* src, std::size_t len)
  : b(src)
  , lim(len)
  , pos(0)
{
}
// Create a view with a bounded underlying buffer of `len` bytes and a read
// offset `pos`.
ByteView::ByteView(std::uint8_t* src, std::size_t len, std::size_t pos)
  : b(src)
  , lim(len)
  , pos(pos)
{
  assert(pos < lim);
}

ByteView ByteView::view(std::size_t pos, std::size_t len) const
{
  const std::size_t limit = pos + len;
  assert(limit <= lim);
  assert(pos < limit);
  return ByteView(b, limit, pos);
}
/* ByteView - C/dtor
 *****************************************************************************/

/******************************************************************************
 * ByteView - Operators
 */
bool ByteView::operator==(const ByteView& rhs) const
{
  std::size_t len = remaining();
  if (rhs.remaining() != len)
    return false;
  const std::uint8_t* lhs_b = cbuf();
  const std::uint8_t* rhs_b = rhs.cbuf();
  while (len-- > 0)
    if (*lhs_b++ != *rhs_b++)
      return false;
  return true;
}
const std::uint8_t& ByteView::operator[](std::size_t i) const
{
  assert(i < lim);
  return b[i];
}
const std::uint8_t& ByteView::operator*() const
{
  assert(pos < lim);
  return b[pos];
}
/* ByteView - Operators
 *****************************************************************************/


/******************************************************************************
 * ByteView - View Modifiers
 */
ByteView& ByteView::seek(std::size_t i)
{
  assert(i < lim);
  pos = i;
  return *this;
}
ByteView& ByteView::limit(std::size_t newlim)
{
  assert(newlim <= lim);
  lim = newlim;
  if (pos > lim)
    pos = lim;
  return *this;
}
/* ByteView - View Modifiers
 *****************************************************************************/

/******************************************************************************
 * ByteView - Reading
 */
std::size_t ByteView::copy_to(void* dst, std::size_t pos, std::size_t len) const
{
  const std::size_t max = remaining();
  if (max < len)
    len = max;
  auto src = static_cast<const void*>(b+pos);
  std::memcpy(dst, src, len);
  return len;
}
std::size_t ByteView::copy_to(void* dst, std::size_t len) const
{
  return copy_to(dst, 0, len);
}

ByteView& ByteView::get(void* dst, std::size_t len)
{
  assert(remaining() >= len);
  copy_to(dst, pos, len);
  pos += len;
  return *this;
}

template <>
std::uint8_t ByteView::get<std::uint8_t>()
{
  assert(remaining() >= 1);
  return b[pos++];
}
template <>
std::uint16_t ByteView::get<std::uint16_t>()
{
  assert(remaining() >= 2);
  auto v = std::uint16_t{b[pos++]} << 8;
  v |= b[pos++];
  return v;
}
template <>
std::uint32_t ByteView::get<std::uint32_t>()
{
  assert(remaining() >= 4);
  auto v = std::uint32_t{b[pos++]} << 24;
  v |= std::uint32_t{b[pos++]} << 16;
  v |= std::uint32_t{b[pos++]} << 8;
  v |= b[pos++];
  return v;
}
template <>
std::uint64_t ByteView::get<std::uint64_t>()
{
  assert(remaining() >= 8);
  auto v = std::uint64_t{b[pos++]} << 56;
  v |= std::uint64_t{b[pos++]} << 48;
  v |= std::uint64_t{b[pos++]} << 40;
  v |= std::uint64_t{b[pos++]} << 32;
  v |= std::uint64_t{b[pos++]} << 24;
  v |= std::uint64_t{b[pos++]} << 16;
  v |= std::uint64_t{b[pos++]} << 8;
  v |= b[pos++];
  return v;
}

/*************************************
 * ByteView readers
 */
template <>
ByteView& ByteView::operator>>(const arrcopy_w<std::uint8_t>& dst)
{
  copy_to(dst.arr, dst.len);
  return *this;
}
template <>
ByteView& ByteView::operator>>(const arrcopy_w<char>& dst)
{
  copy_to(dst.arr, dst.len);
  return *this;
}
template <>
ByteView& ByteView::operator>>(std::uint8_t& v)
{
  assert(remaining() >= 1);
  v = b[pos++];
  return *this;
}
template <>
ByteView& ByteView::operator>>(std::uint16_t& v)
{
  v = get<std::uint16_t>();
  return *this;
}
template <>
ByteView& ByteView::operator>>(std::uint32_t& v)
{
  v = get<std::uint32_t>();
  return *this;
}
template <>
ByteView& ByteView::operator>>(std::uint64_t& v)
{
  v = get<std::uint64_t>();
  return *this;
}


/*************************************
 * ByteBuffer covariant casts
 */
template <>
ByteBuffer& ByteBuffer::operator>>(const arrcopy_w<std::uint8_t>& dst)
{
  ByteView::operator>>(dst);
  return *this;
}
template <>
ByteBuffer& ByteBuffer::operator>>(const arrcopy_w<char>& dst)
{
  ByteView::operator>>(dst);
  return *this;
}
template <>
ByteBuffer& ByteBuffer::operator>>(std::uint8_t& v)
{
  ByteView::operator>>(v);
  return *this;
}
template <>
ByteBuffer& ByteBuffer::operator>>(std::uint16_t& v)
{
  v = get<std::uint16_t>();
  return *this;
}
template <>
ByteBuffer& ByteBuffer::operator>>(std::uint32_t& v)
{
  v = get<std::uint32_t>();
  return *this;
}
template <>
ByteBuffer& ByteBuffer::operator>>(std::uint64_t& v)
{
  v = get<std::uint64_t>();
  return *this;
}
/* ByteView - Reading
 *****************************************************************************/


/******************************************************************************
 * ByteBuffer - Static Factories
 */
ByteBuffer ByteBuffer::allocate(std::size_t len) { return ByteBuffer(len); }
ByteBuffer ByteBuffer::wrap(std::uint8_t* buf, std::size_t len)
{
  return ByteBuffer(buf, len);
}
ByteBuffer ByteBuffer::wrap(char* buf, std::size_t len)
{
  return ByteBuffer(uint8_p(buf), len);
}
ByteBuffer ByteBuffer::copy(const std::uint8_t* buf, std::size_t len)
{
  return ByteBuffer(buf, len);
}
ByteBuffer ByteBuffer::copy(const char* buf, std::size_t len)
{
  return ByteBuffer(uint8_cp(buf), len);
}
/* ByteBuffer- Static Factories
 *****************************************************************************/


/******************************************************************************
 * ByteBuffer - C/dtor / allocator
 */
ByteBuffer::ByteBuffer(ByteBuffer&& rhs)
  : ByteView(rhs.b, rhs.lim, rhs.pos)
  , cap(rhs.cap)
  , owner(rhs.owner)
{
  rhs.b = nullptr;
  rhs.owner = false;
  rhs.pos = rhs.cap = rhs.lim = 0;
}

ByteBuffer::ByteBuffer(std::size_t capacity)
  : ByteView(new std::uint8_t[capacity], capacity)
  , cap(capacity)
  , owner(true)
{
}
ByteBuffer::ByteBuffer(const std::uint8_t* src, std::size_t len)
  : ByteView(new std::uint8_t[len], len)
  , cap(len)
  , owner(true)
{
  std::memcpy(b, src, len);
}
ByteBuffer::ByteBuffer(const ByteBuffer& rhs)
  : ByteView(new std::uint8_t[rhs.cap], rhs.lim, rhs.pos)
  , cap(rhs.cap)
  , owner(true)
{
  std::memcpy(b, rhs.b, cap);
}
ByteBuffer::ByteBuffer(std::uint8_t* buf, std::size_t len)
  : ByteView(buf, len)
  , cap(len)
  , owner(false)
{
}
ByteBuffer::ByteBuffer(std::uint8_t* buf,
                       std::size_t cap,
                       std::size_t lim,
                       std::size_t pos)
  : ByteView(buf, lim, pos)
  , cap(cap)
  , owner(false)
{
}

// Allocate a new buffer equal in len to this and copy this buffer's full
// contents to it.
ByteBuffer ByteBuffer::duplicate() { return ByteBuffer(*this); }

ByteBuffer::~ByteBuffer()
{
  if (owner)
    delete[] b;
}
/* ByteBuffer- C/dtor / allocator
 *****************************************************************************/

/******************************************************************************
 * ByteBuffer - Operators
 */
ByteBuffer& ByteBuffer::operator=(const ByteBuffer& rhs)
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
std::uint8_t& ByteBuffer::operator[](std::size_t i)
{
  assert(i < lim);
  return b[i];
}
std::uint8_t& ByteBuffer::operator*()
{
  assert(pos < lim);
  return b[pos];
}
/* ByteBuffer- Operators
 *****************************************************************************/

/******************************************************************************
 * ByteBuffer - Navigation
 */
ByteBuffer& ByteBuffer::seek(std::size_t i)
{
  ByteView::seek(i);
  return *this;
}
ByteBuffer& ByteBuffer::limit(std::size_t newlim)
{
  assert(newlim <= cap);
  lim = newlim;
  if (pos > lim)
    pos = lim;
  return *this;
}
ByteBuffer& ByteBuffer::flip()
{
  lim = pos;
  pos = 0;
  return *this;
}
ByteBuffer& ByteBuffer::clear()
{
  lim = cap;
  pos = 0;
  return *this;
}
/* ByteBuffer - Navigation
 *****************************************************************************/


/******************************************************************************
 * ByteBuffer - Writing
 */
ByteBuffer& ByteBuffer::put(const std::uint8_t* src, std::size_t len)
{
  assert(remaining() >= len);
  std::memcpy(b + pos, src, len);
  pos += len;
  return *this;
}

template <>
ByteBuffer& ByteBuffer::put(const std::uint8_t& v)
{
  assert(remaining() >= 1);
  b[pos++] = v;
  return *this;
}
template <>
ByteBuffer& ByteBuffer::put(const std::uint16_t& v)
{
  assert(remaining() >= 2);
  b[pos++] = v >> 8;
  b[pos++] = v & 0xFF;
  return *this;
}
template <>
ByteBuffer& ByteBuffer::put(const std::uint32_t& v)
{
  assert(remaining() >= 4);
  b[pos++] = v >> 24;
  b[pos++] = (v >> 16) & 0xFF;
  b[pos++] = (v >> 8) & 0xFF;
  b[pos++] = v & 0xFF;
  return *this;
}
template <>
ByteBuffer& ByteBuffer::put(const std::uint64_t& v)
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
/* ByteBuffer - Writing
 *****************************************************************************/
}
