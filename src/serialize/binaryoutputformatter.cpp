#include <sma/serialize/binaryoutputformatter.hpp>

#include <ostream>
#include <string>

#include <cassert>


namespace sma
{
using Myt = BinaryOutputFormatter;

Myt::BinaryOutputFormatter(std::ostream* os)
  : os(os)
{
}
Myt::BinaryOutputFormatter(Myt&& r)
  : os(r.os)
{
  r.os = nullptr;
}
Myt::BinaryOutputFormatter(Myt const& r)
  : os(r.os)
{
}
Myt& Myt::operator=(Myt&& r)
{
  std::swap(os, r.os);
  return *this;
}
Myt& Myt::operator=(Myt const& r)
{
  os = r.os;
  return *this;
}

template <typename T>
void put_xint(T t, std::ostream* os)
{
  static_assert(
      std::is_integral<T>::value,
      "T must be an integral type for bytewise deconstruction to make sense.");

  auto const size = sizeof(T);

  std::uint8_t buf[size];
  std::size_t sh = size * 8;
  for (std::size_t i = 0; i < size; ++i) {
    sh -= 8;
    buf[i] = (t >> sh) & 0xff;
  }

  os->write(reinterpret_cast<std::istream::char_type*>(buf), size);
  assert(os->good());
}

template <>
Myt& Myt::put(std::int8_t t)
{
  os->put(reinterpret_cast<std::ostream::char_type>(t));
  assert(os->good());
  return *this;
}
template <>
Myt& Myt::put(std::uint8_t t)
{
  os->put(reinterpret_cast<std::ostream::char_type>(t));
  assert(os->good());
  return *this;
}

// clang-format off
template<>
Myt& put(std::int16_t t) { put_xint(t, os); return *this; }
template<>
Myt& put(std::int32_t t) { put_xint(t, os); return *this; }
template<>
Myt& put(std::int64_t t) { put_xint(t, os); return *this; }
template<>
Myt& put(std::uint16_t t) { put_xint(t, os); return *this; }
template<>
Myt& put(std::uint32_t t) { put_xint(t, os); return *this; }
template<>
Myt& put(std::uint64_t t) { put_xint(t, os); return *this; }
// clang-format on


/*
 * This is only portable to architectures with IEEE floats. Sorry, no
 * mainframes!
 */
template <>
Myt& Myt::put(float t)
{
  static_assert(sizeof(float) == sizeof(unsigned int),
                "Your implementation's float isn't the same size as its "
                "unsigned int; I don't know how to serialize floats with this "
                "implementation.");

  unsigned int ui = 0;
  std::memcpy((void*) &ui, (void const*) &f, sizeof(float));
  put_xint(ui, os);
  return *this;
}
template <>
Myt& Myt::put(double t)
{
  static_assert(
      sizeof(double) == sizeof(unsigned long),
      "Your implementation's double isn't the same size as its "
      "unsigned long; I don't know how to serialize doubles with this "
      "implementation.");

  unsigned long ul = 0;
  std::memcpy((void*) &ul, (void const*) &f, sizeof(double));
  put_xint(ul, os);
  return *this;
}


Myt& Myt::put(std::int8_t const* src, std::size_t size)
{
  os->write(reinterpret_cast<std::istream::char_type const*>(src), size);
  assert(os->good());
  return *this;
}
Myt& Myt::put(std::uint8_t const* src, std::size_t size)
{
  os->write(reinterpret_cast<std::istream::char_type const*>(src), size);
  assert(os->good());
  return *this;
}
template <>
Myt& Myt::put(std::string const& s)
{
  static_assert(
      sizeof(std::uint8_t) == sizeof(std::string::value_type),
      "Your implementation's std::string::value_type isn't 8 bits; I "
      "don't know how to serialize strings with this implementation.");

  assert(s.size() <= numeric_limits<std::uint16_t>::max);
  put(static_cast<std::uint16_t>(s.size()));
  put(s.c_str(), s.size());
  return *this;
}
}
