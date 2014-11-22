#include <sma/serialize/binaryformatter.hpp>

#include <string>
#include <cstring>
#include <cassert>
#include <limits>


namespace sma
{
template <typename Stream>
using Myt = BinaryFormatter<Stream>;

template <typename Stream>
Myt::BinaryFormatter(Stream* ios)
  : ios(ios)
{
}
Myt::BinaryFormatter(Myt&& r)
  : ios(r.ios)
{
  r.ios = nullptr;
}
Myt::BinaryFormatter(Myt const& r)
  : ios(r.ios)
{
}
Myt& Myt::operator=(Myt&& r)
{
  std::swap(ios, r.ios);
  return *this;
}
Myt& Myt::operator=(Myt const& r)
{
  ios = r.ios;
  return *this;
}


/******************************************************************************
 * Byte arrays and c-strings
 */
Myt& Myt::write(void const* src, std::size_t size)
{
  // Undefined is just a word
  ios->write(reinterpret_cast<char const*>(src), size);
  assert(ios->good());
  return *this;
}
Myt& Myt::read(void* dst, std::size_t size)
{
  ios->read(reinterpret_cast<char*>(dst), size);
  assert(ios->good());
  return *this;
}


/******************************************************************************
 * std::string
 */
template <>
Myt& Myt::put(std::string const& s)
{
  static_assert(sizeof(std::uint8_t) == sizeof(std::string::value_type),
                "Your implementation's std::string::value_type isn't 8 bits; I "
                "don't know how to serialize strings in this implementation.");

  assert(s.size() <= std::numeric_limits<std::uint16_t>::max());
  put(static_cast<std::uint16_t>(s.size()));
  return write(s.c_str(), s.size());
}
// TODO: Maybe I'll add wide strings. Ha    ha haha hahahahahahaahahahahhaahaha
template <>
std::string Myt::get<std::string>()
{
  static_assert(
      sizeof(std::uint8_t) == sizeof(std::string::value_type),
      "Your implementation's std::string::value_type isn't 8 bits; I "
      "don't know how to deserialize strings with this implementation.");

  // Most strings will fit without an allocation
  std::string::value_type buf[1024];
  std::string::pointer cstr = buf;

  auto size = std::size_t{get<std::uint16_t>()};
  if (size > sizeof buf)
    cstr = new std::string::value_type[size];

  ios->read(cstr, size);
  if (!ios->good() && cstr != buf)
    delete cstr;
  assert(ios->good());

  auto s = std::string(cstr, size);

  if (cstr != buf)
    delete cstr;

  return s;
}
}
