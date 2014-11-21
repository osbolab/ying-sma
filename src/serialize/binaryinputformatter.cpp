#include <sma/serialize/binaryinputformatter.hpp>

#include <istream>
#include <string>

#include <cassert>


namespace sma
{
using Myt = BinaryInputFormatter;

Myt::BinaryInputFormatter(std::istream* is)
  : is(is)
{
}
Myt::BinaryInputFormatter(Myt&& r)
  : is(r.is)
{
  r.is = nullptr;
}
Myt::BinaryInputFormatter(Myt const& r)
  : is(r.is)
{
}
Myt& Myt::operator=(Myt&& r)
{
  std::swap(is, r.is);
  return *this;
}
Myt& Myt::operator=(Myt const& r)
{
  is = r.is;
  return *this;
}

template <typename T>
T get_xint(std::istream* is)
{
  static_assert(
      std::is_integral<T>::value,
      "T must be an integral type for bytewise construction to make sense.");

  auto const size = sizeof(T);

  std::uint8_t buf[size];
  is->read(reinterpret_cast<std::istream::char_type*>(buf), size);
  assert(is->good());

  T t = 0;
  std::size_t sh = size * 8;
  for (std::size_t i = 0; i < size; ++i) {
    sh -= 8;
    t |= T{buf[i]} << sh;
  }
  return t;
}

template <>
std::int8_t Myt::get<std::int8_t>()
{
  auto b = static_cast<std::int8_t>(is.get());
  assert(is->good());
  return b;
}
template <>
std::uint8_t Myt::get<std::uint8_t>()
{
  auto b = static_cast<std::uint8_t>(is.get());
  assert(is->good());
  return b;
}

// clang-format off
template <>
std::int16_t Myt::get<std::int16_t>() { return get_xint<std::int16_t>(is); }
template <>
std::int32_t Myt::get<std::int32_t>() { return get_xint<std::int32_t>(is); }
template <>
std::int64_t Myt::get<std::int64_t>() { return get_xint<std::int64_t>(is); }
template <>
std::uint16_t Myt::get<std::uint16_t>() { return get_xint<std::uint16_t>(is); }
template <>
std::uint32_t Myt::get<std::uint32_t>() { return get_xint<std::uint32_t>(is); }
template <>
std::uint64_t Myt::get<std::uint64_t>() { return get_xint<std::uint64_t>(is); }
// clang-format on

/*
 * This is only portable to architectures with IEEE floats. Sorry, no
 * mainframes!
 * Don't even ask me to support long double portably.
 */
template <>
float Myt::get<float>()
{
  static_assert(
      sizeof(float) == sizeof(unsigned int),
      "Your implementation's float isn't the same size as its "
      "unsigned int; I don't know how to deserialize floats with this "
      "implementation.");

  auto ui = get_xint<unsigned int>(is);
  float f = 0f;
  std::memcpy((void*) &f, (void const*) &ui, sizeof(float));
  return f;
}
template <>
double Myt::get<double>()
{
  static_assert(
      sizeof(double) == sizeof(unsigned long),
      "Your implementation's double isn't the same size as its "
      "unsigned long; I don't know how to deserialize doubles with this "
      "implementation.");

  auto ul = get_xint<unsigned long>(is);
  double d = 0.0;
  std::memcpy((void*) &d, (void const*) &ul, sizeof(double));
  return d;
}


Myt& Myt::get(std::uint8_t* dst, std::size_t size)
{
  is->read(reinterpret_cast<std::istream::char_type*>(dst), size);
  assert(is->good());
  return *this;
}
Myt& Myt::get(std::int8_t* dst, std::size_t size)
{
  is->read(reinterpret_cast<std::istream::char_type*>(dst), size);
  assert(is->good());
  return *this;
}
// TODO: Maybe I'll add wchar strings. Maybe.
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

  std::size_t size = get<std::uint16_t>();
  if (size > sizeof buf)
    cstr = new std::string::value_type[size];

  is->read(cstr, size);
  if (!is->good() && cstr != buf)
    delete cstr;
  assert(is->good());

  auto s = std::string(cstr, size);

  if (cstr != buf)
    delete cstr;

  return s;
}
}
