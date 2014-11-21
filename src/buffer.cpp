#include <sma/Buffer.hpp>
#include <sma/bytes.hpp>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <utility>


namespace sma
{
namespace detail
{
  BufferView::BufferView()
    : b(nullptr)
    , lim(0)
    , pos(0)
  {
  }
  BufferView::BufferView(void* src, std::size_t size)
    : b(static_cast<std::uint8_t*>(src))
    , lim(size)
    , pos(0)
  {
  }
  BufferView::BufferView(void const* src, std::size_t size)
    : BufferView(const_cast<void*>(src), size)
  {
  }
  BufferView::BufferView(void* src, std::size_t size, std::size_t pos)
    : b(static_cast<std::uint8_t*>(src))
    , lim(size)
    , pos(pos)
  {
    assert(pos < lim);
  }

  BufferView BufferView::view(std::size_t pos, std::size_t size) const
  {
    const std::size_t limit = pos + size;
    assert(limit <= lim);
    assert(pos < limit);
    return BufferView(b, limit, pos);
  }


  bool BufferView::operator==(const BufferView& rhs) const
  {
    std::size_t size = remaining();
    if (rhs.remaining() != size)
      return false;
    const std::uint8_t* lhs_b = cbuf();
    const std::uint8_t* rhs_b = rhs.cbuf();
    while (size-- > 0)
      if (*lhs_b++ != *rhs_b++)
        return false;
    return true;
  }
  const std::uint8_t& BufferView::operator[](std::size_t i) const
  {
    assert(i < lim);
    return b[i];
  }
  const std::uint8_t& BufferView::operator*() const
  {
    assert(pos < lim);
    return b[pos];
  }

  std::uint8_t const* BufferView::cbuf() const
  {
    assert(pos < lim);
    return b + pos;
  }

  BufferView& BufferView::seek(std::size_t i)
  {
    assert(i < lim);
    pos = i;
    return *this;
  }
  BufferView& BufferView::limit(std::size_t newlim)
  {
    assert(newlim <= lim);
    lim = newlim;
    if (pos > lim)
      pos = lim;
    return *this;
  }

  std::size_t
  BufferView::copy_to(void* dst, std::size_t size, std::size_t pos) const
  {
    auto max = remaining();
    if (max < size)
      size = max;
    auto src = static_cast<const void*>(b + pos);
    std::memcpy(dst, src, size);
    return size;
  }
  std::size_t BufferView::copy_to(void* dst, std::size_t size) const
  {
    return copy_to(dst, 0, size);
  }

  void* BufferView::get(void* dst, std::size_t size)
  {
    assert(remaining() >= size);
    copy_to(dst, size, pos);
    pos += size;
    return dst;
  }
  template <>
  std::uint8_t BufferView::get<std::uint8_t>()
  {
    assert(remaining() >= 1);
    return b[pos++];
  }
  template <>
  std::uint16_t BufferView::get<std::uint16_t>()
  {
    assert(remaining() >= 2);
    auto v = std::uint16_t{b[pos++]} << 8;
    v |= b[pos++];
    return v;
  }
  template <>
  std::uint32_t BufferView::get<std::uint32_t>()
  {
    assert(remaining() >= 4);
    auto v = std::uint32_t{b[pos++]} << 24;
    v |= std::uint32_t{b[pos++]} << 16;
    v |= std::uint32_t{b[pos++]} << 8;
    v |= b[pos++];
    return v;
  }
  template <>
  std::uint64_t BufferView::get<std::uint64_t>()
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

  template <>
  BufferView& BufferView::operator>>(std::uint8_t& dst)
  {
    assert(remaining() >= 1);
    dst = b[pos++];
    return *this;
  }
  BufferView& BufferView::operator>>(std::uint16_t& dst)
  {
    dst = get<std::uint16_t>();
    return *this;
  }
  BufferView& BufferView::operator>>(std::uint32_t& dst)
  {
    dst = get<std::uint32_t>();
    return *this;
  }
  BufferView& BufferView::operator>>(std::uint64_t& dst)
  {
    dst = get<std::uint64_t>();
    return *this;
  }
}    // detail


Buffer Buffer::allocate(std::size_t size) { return Buffer(size); }
Buffer Buffer::wrap(void* buf, std::size_t size)
{
  return Buffer(static_cast<std::uint8_t*>(buf), size);
}
Buffer Buffer::copy(void const* buf, std::size_t size)
{
  return Buffer(static_cast<std::uint8_t const*>(buf), size);
}


Buffer::Buffer(Buffer&& rhs)
  : Buffer::view(std::move(rhs.b), rhs.lim, rhs.pos)
  , owner(rhs.owner)
  , cap(rhs.cap)
{
  rhs.b = nullptr;
  rhs.owner = false;
  rhs.cap = rhs.lim = rhs.pos = 0;
}
Buffer::Buffer(Buffer const& rhs)
  : Buffer::view(new std::uint8_t[rhs.cap], rhs.lim, rhs.pos)
  , cap(rhs.cap)
  , owner(true)
{
  std::memcpy(b, rhs.b, cap);
}
Buffer::Buffer(std::size_t capacity)
  : Buffer::view(new std::uint8_t[capacity], capacity)
  , cap(capacity)
  , owner(true)
{
}
Buffer::Buffer(std::uint8_t const* src, std::size_t size)
  : Buffer::view(new std::uint8_t[size], size)
  , cap(size)
  , owner(true)
{
  std::memcpy(b, src, size);
}
Buffer::Buffer(std::uint8_t* buf, std::size_t size)
  : Buffer::view(buf, size)
  , cap(size)
  , owner(false)
{
}
Buffer::Buffer(std::uint8_t* buf,
               std::size_t cap,
               std::size_t lim,
               std::size_t pos)
  : Buffer::view(buf, lim, pos)
  , cap(cap)
  , owner(false)
{
}

Buffer::~Buffer()
{
  if (owner)
    delete[] b;
}

Buffer& Buffer::operator=(Buffer&& rhs)
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
Buffer& Buffer::operator=(Buffer const& rhs)
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
std::uint8_t& Buffer::operator[](std::size_t i)
{
  assert(i < lim);
  return b[i];
}
std::uint8_t& Buffer::operator*()
{
  assert(pos < lim);
  return b[pos];
}

std::uint8_t* buf();
{
  assert(pos < cap);
  return b + pos;
}
Buffer& Buffer::seek(std::size_t i)
{
  Buffer::view::seek(i);
  return *this;
}
Buffer& Buffer::limit(std::size_t newlim)
{
  assert(newlim <= cap);
  lim = newlim;
  if (pos > lim)
    pos = lim;
  return *this;
}
Buffer& Buffer::flip()
{
  lim = pos;
  pos = 0;
  return *this;
}
Buffer& Buffer::clear()
{
  lim = cap;
  pos = 0;
  return *this;
}

Buffer& Buffer::operator>>(std::uint8_t& dst)
{
  dst = get_uint8();
  return *this;
}
Buffer& Buffer::operator>>(std::uint16_t& dst)
{
  dst = get_uint16();
  return *this;
}
Buffer& Buffer::operator>>(std::uint32_t& dst)
{
  dst = get_uint32();
  return *this;
}
Buffer& Buffer::operator>>(std::uint64_t& dst)
{
  dst = get_uint64();
  return *this;
}

Buffer& Buffer::replace(void const* src, std::size_t size)
{
  assert(size >= cap);
  std::memcpy(b, src, cap);
  clear();
  return *this;
}
Buffer& Buffer::put(void const* src, std::size_t size)
{
  assert(remaining() >= size);
  std::memcpy(b + pos, src, size);
  pos += size;
  return *this;
}
Buffer& Buffer::put_8(std::uint8_t const& src)
{
  assert(remaining() >= 1);
  b[pos++] = src;
  return *this;
}
Buffer& Buffer::put_16(std::uint16_t const& src)
{
  assert(remaining() >= 2);
  b[pos++] = src >> 8;
  b[pos++] = src & 0xFF;
  return *this;
}
Buffer& Buffer::put_32(std::uint32_t const& src)
{
  assert(remaining() >= 4);
  b[pos++] = src >> 24;
  b[pos++] = (src >> 16) & 0xFF;
  b[pos++] = (src >> 8) & 0xFF;
  b[pos++] = src & 0xFF;
  return *this;
}
Buffer& Buffer::put_64(std::uint64_t const& src)
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

Buffer& Buffer::operator<<(std::uint8_t const& src)
{
  put_8(src);
  return *this;
}
Buffer& Buffer::operator<<(std::uint16_t const& src)
{
  put_16(src);
  return *this;
}
Buffer& Buffer::operator<<(std::uint32_t const& src)
{
  put_32(src);
  return *this;
}
Buffer& Buffer::operator<<(std::uint64_t const& src)
{
  put_64(src);
  return *this;
}
}
