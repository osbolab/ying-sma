#include "bytebuf.hh"

#include <cstdint>
#include <cstring>


namespace sma {
namespace detail {
/******************************************************************************
 * byteview - Static Factories
 */
byteview byteview::wrap(const std::uint8_t* src, std::size_t len)
{
  return byteview(const_cast<std::uint8_t*>(src), len, 0);
}
/* byteview - Static Factories
 *****************************************************************************/

/******************************************************************************
 * byteview - C/dtor
 */

byteview::byteview()
  : b(EMPTY)
  , lim(0)
  , pos(0)
{
}
byteview::byteview(void* src, std::size_t len)
  : b(static_cast<std::uint8_t*>(src))
  , lim(len)
  , pos(0)
{
}
// Create a view with a bounded underlying buffer of `len` bytes and a read
// offset `pos`.
byteview::byteview(void* src, std::size_t len, std::size_t pos)
  : b(static_cast<std::uint8_t*>(src))
  , lim(len)
  , pos(pos)
{
  assert(pos < lim);
}

byteview byteview::view(std::size_t pos, std::size_t len) const
{
  const std::size_t limit = pos + len;
  assert(limit <= lim);
  assert(pos < limit);
  return byteview(b, limit, pos);
}
/* byteview - C/dtor
 *****************************************************************************/

/******************************************************************************
 * byteview - Operators
 */
bool byteview::operator==(const byteview& rhs) const
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
const std::uint8_t& byteview::operator[](std::size_t i) const
{
  assert(i < lim);
  return b[i];
}
const std::uint8_t& byteview::operator*() const
{
  assert(pos < lim);
  return b[pos];
}
/* byteview - Operators
 *****************************************************************************/


/******************************************************************************
 * byteview - View Modifiers
 */
byteview& byteview::seek(std::size_t i)
{
  assert(i < lim);
  pos = i;
  return *this;
}
byteview& byteview::limit(std::size_t newlim)
{
  assert(newlim <= lim);
  lim = newlim;
  if (pos > lim)
    pos = lim;
  return *this;
}
/* byteview - View Modifiers
 *****************************************************************************/

/******************************************************************************
 * byteview - Reading
 */
std::size_t byteview::copy_to(void* dst, std::size_t len, std::size_t pos) const
{
  const std::size_t max = remaining();
  if (max < len)
    len = max;
  auto src = static_cast<const void*>(b+pos);
  std::memcpy(dst, src, len);
  return len;
}
std::size_t byteview::copy_to(void* dst, std::size_t len) const
{
  return copy_to(dst, 0, len);
}

byteview& byteview::get(void* dst, std::size_t len)
{
  assert(remaining() >= len);
  copy_to(dst, pos, len);
  pos += len;
  return *this;
}

template <>
std::uint8_t byteview::get<std::uint8_t>()
{
  assert(remaining() >= 1);
  return b[pos++];
}
template <>
std::uint16_t byteview::get<std::uint16_t>()
{
  assert(remaining() >= 2);
  auto v = std::uint16_t{b[pos++]} << 8;
  v |= b[pos++];
  return v;
}
template <>
std::uint32_t byteview::get<std::uint32_t>()
{
  assert(remaining() >= 4);
  auto v = std::uint32_t{b[pos++]} << 24;
  v |= std::uint32_t{b[pos++]} << 16;
  v |= std::uint32_t{b[pos++]} << 8;
  v |= b[pos++];
  return v;
}
template <>
std::uint64_t byteview::get<std::uint64_t>()
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
 * byteview readers
 */
template <>
byteview& byteview::operator>>(const arrcopy_w<std::uint8_t>& dst)
{
  copy_to(dst.arr, dst.len);
  return *this;
}
template <>
byteview& byteview::operator>>(const arrcopy_w<char>& dst)
{
  copy_to(dst.arr, dst.len);
  return *this;
}
template <>
byteview& byteview::operator>>(const arrcopy_w<void>& dst)
{
  copy_to(dst.arr, dst.len);
  return *this;
}
template <>
byteview& byteview::operator>>(std::uint8_t& v)
{
  assert(remaining() >= 1);
  v = b[pos++];
  return *this;
}
template <>
byteview& byteview::operator>>(std::uint16_t& v)
{
  v = get<std::uint16_t>();
  return *this;
}
template <>
byteview& byteview::operator>>(std::uint32_t& v)
{
  v = get<std::uint32_t>();
  return *this;
}
template <>
byteview& byteview::operator>>(std::uint64_t& v)
{
  v = get<std::uint64_t>();
  return *this;
}

/* byteview - Reading
 *****************************************************************************/

}
}
