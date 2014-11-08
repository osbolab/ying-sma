#include "bytebuffer.hh"

#include <cstdint>


namespace sma
{

bool ReadableByteBuffer::operator==(const ReadableByteBuffer& rhs) const
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

std::size_t ReadableByteBuffer::get(std::uint8_t* dst, std::size_t len)
{
  if (remaining() < len)
    len = remaining();
  const std::uint8_t* src = cbuf();
  while (len-- > 0)
    *dst++ = *src++;
  return src - cbuf();
}

template <>
std::uint8_t ReadableByteBuffer::get<std::uint8_t>()
{
  return b[pos++];
}
template <>
std::uint16_t ReadableByteBuffer::get<std::uint16_t>()
{
  auto v = std::uint16_t{b[pos++]} << 8;
  v |= b[pos++];
  return v;
}
template <>
std::uint32_t ReadableByteBuffer::get<std::uint32_t>()
{
  auto v = std::uint32_t{b[pos++]} << 24;
  v |= std::uint32_t{b[pos++]} << 16;
  v |= std::uint32_t{b[pos++]} << 8;
  v |= b[pos++];
  return v;
}
template <>
std::uint64_t ReadableByteBuffer::get<std::uint64_t>()
{
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
ReadableByteBuffer& ReadableByteBuffer::operator>>(std::uint8_t& v)
{
  v = b[pos++];
  return *this;
}

template <>
ByteBuffer& ByteBuffer::operator>>(std::uint8_t& v)
{
  v = b[pos++];
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

template <>
std::size_t ByteBuffer::put(const std::uint8_t& v)
{
  b[pos++] = v;
  return 1;
}
template <>
std::size_t ByteBuffer::put(const std::uint16_t& v)
{
  b[pos++] = v >> 8;
  b[pos++] = v & 0xFF;
  return 2;
}
template <>
std::size_t ByteBuffer::put(const std::uint32_t& v)
{
  b[pos++] = v >> 24;
  b[pos++] = (v >> 16) & 0xFF;
  b[pos++] = (v >> 8) & 0xFF;
  b[pos++] = v & 0xFF;
  return 4;
}
template <>
std::size_t ByteBuffer::put(const std::uint64_t& v)
{
  b[pos++] = v >> 56;
  b[pos++] = (v >> 48) & 0xFF;
  b[pos++] = (v >> 40) & 0xFF;
  b[pos++] = (v >> 32) & 0xFF;
  b[pos++] = (v >> 24) & 0xFF;
  b[pos++] = (v >> 16) & 0xFF;
  b[pos++] = (v >> 8) & 0xFF;
  b[pos++] = v & 0xFF;
  return 8;
}
}
