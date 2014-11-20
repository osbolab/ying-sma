#include <sma/buffer.hpp>

#include <cstdint>
#include <cstring>


namespace sma
{
namespace detail
{
  /******************************************************************************
   * buffer_view - Static Factories
   */
  buffer_view buffer_view::of(const std::uint8_t* src, std::size_t len)
  {
    return buffer_view(const_cast<std::uint8_t*>(src), len, 0);
  }
  /* buffer_view - Static Factories
   *****************************************************************************/

  /******************************************************************************
   * buffer_view - C/dtor
   */

  buffer_view::buffer_view()
    : b(nullptr)
    , lim(0)
    , pos(0)
  {
  }
  buffer_view::buffer_view(void* src, std::size_t len)
    : b(static_cast<std::uint8_t*>(src))
    , lim(len)
    , pos(0)
  {
  }
  // Create a view with a bounded underlying buffer of `len` bytes and a read
  // offset `pos`.
  buffer_view::buffer_view(void* src, std::size_t len, std::size_t pos)
    : b(static_cast<std::uint8_t*>(src))
    , lim(len)
    , pos(pos)
  {
    assert(pos < lim);
  }

  buffer_view buffer_view::view(std::size_t pos, std::size_t len) const
  {
    const std::size_t limit = pos + len;
    assert(limit <= lim);
    assert(pos < limit);
    return buffer_view(b, limit, pos);
  }
  /* buffer_view - C/dtor
   *****************************************************************************/

  /******************************************************************************
   * buffer_view - Operators
   */
  bool buffer_view::operator==(const buffer_view& rhs) const
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
  const std::uint8_t& buffer_view::operator[](std::size_t i) const
  {
    assert(i < lim);
    return b[i];
  }
  const std::uint8_t& buffer_view::operator*() const
  {
    assert(pos < lim);
    return b[pos];
  }
  /* buffer_view - Operators
   *****************************************************************************/


  /******************************************************************************
   * buffer_view - View Modifiers
   */
  buffer_view& buffer_view::seek(std::size_t i)
  {
    assert(i < lim);
    pos = i;
    return *this;
  }
  buffer_view& buffer_view::limit(std::size_t newlim)
  {
    assert(newlim <= lim);
    lim = newlim;
    if (pos > lim)
      pos = lim;
    return *this;
  }
  /* buffer_view - View Modifiers
   *****************************************************************************/

  /******************************************************************************
   * buffer_view - Reading
   */
  std::size_t
  buffer_view::copy_to(void* dst, std::size_t len, std::size_t pos) const
  {
    const std::size_t max = remaining();
    if (max < len)
      len = max;
    auto src = static_cast<const void*>(b + pos);
    std::memcpy(dst, src, len);
    return len;
  }
  std::size_t buffer_view::copy_to(void* dst, std::size_t len) const
  {
    return copy_to(dst, 0, len);
  }

  void* buffer_view::get(void* dst, std::size_t len)
  {
    assert(remaining() >= len);
    copy_to(dst, len, pos);
    pos += len;
    return dst;
  }
  std::uint8_t buffer_view::get_uint8()
  {
    assert(remaining() >= 1);
    return b[pos++];
  }
  std::uint16_t buffer_view::get_uint16()
  {
    assert(remaining() >= 2);
    auto v = std::uint16_t{b[pos++]} << 8;
    v |= b[pos++];
    return v;
  }
  std::uint32_t buffer_view::get_uint32()
  {
    assert(remaining() >= 4);
    auto v = std::uint32_t{b[pos++]} << 24;
    v |= std::uint32_t{b[pos++]} << 16;
    v |= std::uint32_t{b[pos++]} << 8;
    v |= b[pos++];
    return v;
  }
  std::uint64_t buffer_view::get_uint64()
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
   * buffer_view readers
   */
  buffer_view& buffer_view::operator>>(std::uint8_t& dst)
  {
    assert(remaining() >= 1);
    dst = b[pos++];
    return *this;
  }
  buffer_view& buffer_view::operator>>(std::uint16_t& dst)
  {
    dst = get_uint16();
    return *this;
  }
  buffer_view& buffer_view::operator>>(std::uint32_t& dst)
  {
    dst = get_uint32();
    return *this;
  }
  buffer_view& buffer_view::operator>>(std::uint64_t& dst)
  {
    dst = get_uint64();
    return *this;
  }

  /* buffer_view - Reading
   *****************************************************************************/
}
}
