#pragma once

#include <detail/uint_with_size.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <ostream>

namespace sma
{
struct BinaryStreamWriter {
  BinaryStreamWriter(std::ostream* os)
    : os(os)
  {
  }

  void write(void const* src, std::size_t size)
  {
    os->write(reinterpret_cast<char const*>(src), size);
  }

  template <typename T>
  BinaryStreamWriter& operator<<(T const& t)
  {
    put(t);
    return *this;
  }

  template <typename T>
  void put(T const& t)
  {
    t.write_fields(*this);
  }

  template <typename T>
  void put(std::vector<T> const& v)
  {
    put(std::uint8_t(v.size()));
    for (auto& t : v)
      put(t);
  }

private:
  std::ostream* os;
};

template <>
void BinaryStreamWriter::put<std::string>(std::string const& t)
{
  static_assert(
      sizeof(std::uint8_t) == sizeof(std::string::value_type),
      "std::string isn't std::basic_stream<char> or char isn't 8 bits.");

  auto size = t.size();
  // 2^15 - 1, or two bytes with the highest bit as the extension flag
  assert(size <= 32767);
  // 2^7 - 1, or one byte with the highest bit as the extension flag
  if (size >= 127)
    put(std::uint16_t(size | (1 << 15)));
  else
    put(std::uint8_t(size));

  os->write(t.c_str(), size);
  return *this;
}

template <>
void BinaryStreamWriter::put<float>(float const& t)
{
  typename detail::uint_with_size<sizeof(float)>::type i = 0;
  std::memcpy(&i, &t, sizeof i);
  put(i);
  return *this;
}

template <>
void BinaryStreamWriter::put<double>(double const& t)
{
    typename detail::uint_with_size<sizeof(double)>::type i = 0;
    std::memcpy(&i, &t, sizeof i);
    put(i);
    return *this;
}

template <>
void BinaryStreamWriter::put<std::uint8_t>(std::uint8_t const& t)
{
    ios->write(reinterpret_cast<char const*>(&t), 1);
    return *this;
}

template <>
void BinaryStreamWriter::put<std::uint16_t>(std::uint16_t const& t)
{
    char buf[]{char(t >> 8 & 0xff), char(t & 0xff)};
    ios->write(buf, sizeof(buf));
    return *this;
}

template <>
void BinaryStreamWriter::put<std::uint32_t>(std::uint32_t const& t)
{
    char buf[]{char(t >> 24 & 0xff),
               char(t >> 16 & 0xff),
               char(t >> 8 & 0xff),
               char(t & 0xff)};
    ios->write(buf, sizeof(buf));
    return *this;
}

template <>
void BinaryStreamWriter::put<std::uint64_t>(std::uint64_t const& t)
{
    char buf[]{char(t >> 56 & 0xff),
               char(t >> 48 & 0xff),
               char(t >> 40 & 0xff),
               char(t >> 32 & 0xff),
               char(t >> 24 & 0xff),
               char(t >> 16 & 0xff),
               char(t >> 8 & 0xff),
               char(t & 0xff)};
    ios->write(buf, sizeof(buf));
    return *this;
}
}
