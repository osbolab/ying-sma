#pragma once

#include <ostream>

namespace sma
{
class BinaryOutputFormatter
{
  static_assert(sizeof(std::uint8_t) == sizeof(std::ostream::char_type),
                "std::istream::char_type isn't 8 bits; I don't know how to "
                "serialize with this implementation.");

  using Myt = BinaryOutputFormatter;

public:
  inline BinaryOutputFormatter(std::ostream* os);

  inline BinaryOutputFormatter(Myt&& r);
  inline BinaryOutputFormatter(Myt const& r);

  inline Myt& operator=(Myt&& r);
  inline Myt& operator=(Myt const& r);

  template <typename T>
  Myt& put(T t);
  Myt& put(char const* src, std::size_t size);
  Myt& put(std::uint8_t const* src, std::size_t size);

private:
  std::ostream* os{nullptr};
};
}
