#pragma once

#include <string>
#include <istream>
#include <type_traits>

namespace sma
{
// clang-format off
template <typename Stream = std::iostream>
class BinaryFormatter
{
  using Myt = BinaryFormatter;
  using char_type = typename Stream::char_type;
  using traits_type = typename Stream::traits_type;


  static_assert(sizeof(std::uint8_t) == sizeof(char_type),
                "std::iostream::char_type isn't 8 bits; I don't know how to "
                "format binary data with this implementation.");

public:
  template <typename T, typename Stream> Myt& put(T t, Stream* os);
  template <typename T, typename Stream>    T get(Stream* is);

  Myt& write(void const*  src, std::size_t size);
  Myt&  read(void*        dst, std::size_t size);
};


/******************************************************************************
 * Single-char
 */
template <typename T>
  std::enable_if<
    sizeof(T) == sizeof(char_type)
  , BinaryFormatter&
  >::type
BinaryFormatter::put(T t)
{
  ios->put(char_type{t});
  assert(ios->good());
  return *this;
}
template <typename T>
  std::enable_if<
    sizeof(T) == sizeof(char_type)
  , T
  >::type
BinaryFormatter::get()
{
  auto i = ios->get();
  assert(ios->good());
  return T{typename traits_type::to_char_type(i)};
}


/******************************************************************************
 * Arbitrary-width integral
 */
template <typename T>
  std::enable_if<
    std::is_integral<T>::value
  , BinaryFormatter&
  >::type
BinaryFormatter::put(T t)
{
  auto const size = sizeof(T);

  std::uint8_t buf[size];
  std::size_t sh = size * 8;
  for (std::size_t i = 0; i < size; ++i) {
    sh -= 8;
    buf[i] = (t >> sh) & 0xff;
  }

  ios->write(reinterpret_cast<char_type*>(buf), size);
  assert(ios->good());

  return *this;
}

template <typename T>
  std::enable_if<
    std::is_integral<T>::value
  , T
  >::type
BinaryFormatter::get()
{
  auto const size = sizeof(T);

  std::uint8_t buf[size];
  ios->read(reinterpret_cast<char_type*>(buf), size);
  assert(ios->good());

  T t = 0;
  std::size_t sh = size * 8;
  for (std::size_t i = 0; i < size; ++i) {
    sh -= 8;
    t |= T{buf[i]} << sh;
  }
  return t;
}

/******************************************************************************
 * Floating point
 * This is only portable to architectures with IEEE floats.
 */
template <typename T>
  std::enable_if<
    std::is_floating_point<T>::value
  , BinaryFormatter&
  >::type
BinaryFormatter::put(T f)
{
  typename detail::integral_with_size<sizeof(T)>::type i = 0;
  std::memcpy(&i, &f, sizeof i);
  return put(i);
}

template <typename T>
  std::enable_if<
    std::is_floating_point<T>::value
  , T
  >::type
BinaryFormatter::get()
{
  typedef typename detail::integral_with_size<sizeof(T)>::type integral_type;
  auto i = get<integral_type>();
  T t;
  std::memcpy(&t, &i, sizeof t);
  return t;
}


/******************************************************************************
 * Find integral types with the right size to pun floating point types.
 */
namespace detail
{
template <std::size_t Size>
struct integral_with_size {
  static_assert(false, "No integral type with that size is available.")
};
template <> struct integral_with_size<sizewith(std::uint8_t)>  { using type = std::uint8_t; };
template <> struct integral_with_size<sizewith(std::uint16_t)> { using type = std::uint16_t; };
template <> struct integral_with_size<sizewith(std::uint32_t)> { using type = std::uint32_t; };
template <> struct integral_with_size<sizewith(std::uint64_t)> { using type = std::uint64_t; };
}
// clang-format on
}
