#include <sma/util/binaryoutput.hpp>

#include <cstring>
#include <cassert>

namespace sma
{
BinaryOutput& BinaryOutput::operator<<(std::string const& t)
{
  static_assert(
      sizeof(std::uint8_t) == sizeof(std::string::value_type),
      "std::string isn't std::basic_stream<char> or char isn't 8 bits.");

  auto size = t.size();
  // 2^15 - 1, or two bytes with the highest bit as the extension flag
  assert(size <= 32767);
  // 2^7 - 1, or one byte with the highest bit as the extension flag
  if (size >= 127)
    *this << std::uint16_t(size | (1 << 15));
  else
    *this << std::uint8_t(size);

  os->write(t.c_str(), size);
  return *this;
}

BinaryOutput& BinaryOutput::operator<<(float const& t)
{
  typename detail::uint_with_size<sizeof(float)>::type i = 0;
  std::memcpy(&i, &t, sizeof i);
  *this << i;
  return *this;
}

BinaryOutput& BinaryOutput::operator<<(double const& t)
{
  typename detail::uint_with_size<sizeof(double)>::type i = 0;
  std::memcpy(&i, &t, sizeof i);
  *this << i;
  return *this;
}

BinaryOutput& BinaryOutput::operator<<(std::uint8_t const& t)
{
  os->write(reinterpret_cast<char const*>(&t), 1);
  return *this;
}

BinaryOutput& BinaryOutput::operator<<(std::uint16_t const& t)
{
  char buf[]{char(t >> 8 & 0xff), char(t & 0xff)};
  os->write(buf, sizeof(buf));
  return *this;
}

BinaryOutput& BinaryOutput::operator<<(std::uint32_t const& t)
{
  char buf[]{char(t >> 24 & 0xff),
             char(t >> 16 & 0xff),
             char(t >> 8 & 0xff),
             char(t & 0xff)};
  os->write(buf, sizeof(buf));
  return *this;
}

BinaryOutput& BinaryOutput::operator<<(std::uint64_t const& t)
{
  char buf[]{char(t >> 56 & 0xff),
             char(t >> 48 & 0xff),
             char(t >> 40 & 0xff),
             char(t >> 32 & 0xff),
             char(t >> 24 & 0xff),
             char(t >> 16 & 0xff),
             char(t >> 8 & 0xff),
             char(t & 0xff)};
  os->write(buf, sizeof(buf));
  return *this;
}
}
