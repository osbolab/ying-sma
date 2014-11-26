#pragma once

#include <string>
#include <cstring>
#include <cassert>
#include <limits>
#include <type_traits>

namespace sma
{
// clang-format off

namespace detail
{
  /****************************************************************************
   * Find the integral type with the given size in chars.
   * Used to find a suitable uint for punning floating point types.
   */
  template <std::size_t Size> struct uint_with_size;
  template <> struct uint_with_size<sizeof(std::uint8_t)>  { typedef std::uint8_t type; };
  template <> struct uint_with_size<sizeof(std::uint16_t)> { typedef std::uint16_t type; };
  template <> struct uint_with_size<sizeof(std::uint32_t)> { typedef std::uint32_t type; };
  template <> struct uint_with_size<sizeof(std::uint64_t)> { typedef std::uint64_t type; };
}

// This class is such a mess I had to turn the formatter off for the whole thing.
// Please don't judge me

template <typename Stream>
class BinaryFormatter
{
  static_assert(sizeof(std::uint8_t) == sizeof(char),
                "Don't know how to format with a non-8-bit char");


  using Myt = BinaryFormatter<Stream>;

  Stream* ios;

public:
  BinaryFormatter(Stream* ios) : ios(ios) {}

  // 'Undefined' and 'behavior' are just words
  Myt& write(void const* src, std::size_t size)
    { ios->write(reinterpret_cast<char const*>(src), size); return *this; }

  Myt& read(void* dst, std::size_t size)
    { ios->read(reinterpret_cast<char*>(dst), size); return *this; }

  template <typename T>
    Myt& operator<<(T t)
    { put(std::forward<T>(t));               return *this; }

  template <typename T>
    Myt& operator>>(T& t)
    { t = get<T>();         return *this; }

  template <typename T>
    Myt& put(T t)
    { t.write_fields(this); return *this; }

  Myt& put(char t)
    { ios->write(&t, 1);    return *this; }

  Myt& put(std::uint8_t t)
    { ios->write(reinterpret_cast<char const*>(&t), 1); return *this; }

  Myt& put(std::uint16_t t)
    {
      char buf[] {
        char(t >> 8 & 0xff)
      , char(t      & 0xff)
      };
      ios->write(buf, sizeof(buf));
      return *this;
    }

  Myt& put(std::uint32_t t)
    {
      char buf[] {
        char(t >> 24 & 0xff)
      , char(t >> 16 & 0xff)
      , char(t >> 8  & 0xff)
      , char(t       & 0xff)
      };
      ios->write(buf, sizeof(buf));
      return *this;
    }

  Myt& put(std::uint64_t t)
    {
      char buf[] {
        char(t >> 56 & 0xff)
      , char(t >> 48 & 0xff)
      , char(t >> 40 & 0xff)
      , char(t >> 32 & 0xff)
      , char(t >> 24 & 0xff)
      , char(t >> 16 & 0xff)
      , char(t >> 8  & 0xff)
      , char(t       & 0xff)
      };
      ios->write(buf, sizeof(buf));
      return *this;
    }

  /******************************************************************************
   * This is only portable to architectures with IEEE floats.
   */
  Myt& put(float  t)
    {
      typename detail::uint_with_size<sizeof(float)>::type i = 0;
      std::memcpy(&i, &t, sizeof i);
      put(i);
      return *this;
    }

  Myt& put(double t)
    {
      typename detail::uint_with_size<sizeof(double)>::type i = 0;
      std::memcpy(&i, &t, sizeof i);
      put(i);
      return *this;
    }

  // This is just... so bad... I'm so sorry

  template <typename T>
    typename std::enable_if<std::is_constructible<T, Myt*>::value,
      T>::type get()
    {
      return T(this);
    }

  template <typename T>
    typename std::enable_if<std::is_same<T, char>::value,
      T>::type get()
    {
      char buf[1];
      ios->read(buf, 1);
    return buf[0];
    }

  template <typename T>
    typename std::enable_if<std::is_same<T, std::uint8_t>::value,
      T>::type get()
    {
      char buf[1];
      ios->read(buf, 1);
      return std::uint8_t(buf[0]);
    }

  template <typename T>
    typename std::enable_if<std::is_same<T, std::uint16_t>::value,
      T>::type get()
    {
      std::uint8_t buf[2];
      ios->read(reinterpret_cast<char*>(buf), sizeof(buf));
      return std::uint16_t{buf[0]} << 8
           | std::uint16_t{buf[1]};
    }

  template <typename T>
    typename std::enable_if<std::is_same<T, std::uint32_t>::value,
      T>::type get()
    {
      std::uint8_t buf[4];
      ios->read(reinterpret_cast<char*>(buf), sizeof(buf));
      return std::uint32_t{buf[0]} << 24
           | std::uint32_t{buf[1]} << 16
           | std::uint32_t{buf[2]} << 8
           | std::uint32_t{buf[3]};
    }

  template <typename T>
    typename std::enable_if<std::is_same<T, std::uint64_t>::value,
      T>::type get()
    {
      std::uint8_t buf[8];
      ios->read(reinterpret_cast<char*>(buf), sizeof(buf));
      return std::uint64_t{buf[0]} << 56
           | std::uint64_t{buf[1]} << 48
           | std::uint64_t{buf[2]} << 40
           | std::uint64_t{buf[3]} << 32
           | std::uint64_t{buf[4]} << 24
           | std::uint64_t{buf[5]} << 16
           | std::uint64_t{buf[6]} << 8
           | std::uint64_t{buf[7]};
    }

  template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value,
      T>::type get()
    {
      typedef typename detail::uint_with_size<sizeof(T)>::type I;
      auto i = get();
      T t;
      std::memcpy(&t, &i, sizeof t);
      return t;
    }


  // TODO: Maybe I'll add wide strings. Ha    ha haha hahahahahahaahahahahhaahahaaah hahaha
  Myt& put(std::string const& s)
    {
      static_assert(
          sizeof(std::uint8_t) == sizeof(std::string::value_type),
          "Your implementation's std::string doesn't use 8-bit characters. "
          "I don't know how to serialize strings in this implementation.");

      std::size_t size = s.size();
      // 2^15 - 1, or two bytes with the highest bit as the extension flag
      assert(size <= 32767);
      // 2^7 - 1, or one byte with the highest bit as the extension flag
      if (size >= 127)
        put(std::uint16_t(size | (1 << 15)));
      else
        put(std::uint8_t(size));

      ios->write(s.c_str(), size);
      return *this;
    }

  template <typename T>
    typename std::enable_if<std::is_same<T, std::string>::value,
      T>::type get()
    {
      static_assert(
          sizeof(std::uint8_t) == sizeof(std::string::value_type),
          "Your implementation's std::string doesn't use 8-bit characters. "
          "I don't know how to deserialize strings with this implementation.");

      // Most strings will fit without an allocation
      std::string::value_type buf[1024];
      std::string::pointer cstr = buf;

      // If the high bit of the first byte is set then it's a two byte size;
      // unset the high bit and shift in the next byte as the low byte.
      // This gives us a range of 127 bytes with extension to 32,767 bytes
      // -- way more than enough.
      std::size_t size = get<std::uint8_t>();
      if (size & 0x80)
        // Remove the flag bit and fetch the low byte
        size = (size & ~0x80) << 8 | get<std::uint8_t>();

      if (size > sizeof buf)
        cstr = new std::string::value_type[size];

      ios->read(cstr, size);
      auto s = std::string(cstr, size);

      if (cstr != buf)
        delete cstr;

      return s;
    }
};

// clang-format on
}
