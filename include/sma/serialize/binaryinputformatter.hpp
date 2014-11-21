#pragma once

#include <istream>

namespace sma
{
class BinaryInputFormatter
{
  static_assert(sizeof(std::uint8_t) == sizeof(std::istream::char_type),
                "std::istream::char_type isn't 8 bits; I don't know how to "
                "deserialize with this implementation.");

  using Myt = BinaryInputFormatter;

public:
  inline BinaryInputFormatter(std::istream* is);

  inline BinaryInputFormatter(Myt&& r);
  inline BinaryInputFormatter(Myt const& r);

  inline Myt& operator=(Myt&& r);
  inline Myt& operator=(Myt const& r);

  template <typename T>
  T get();
  Myt& get(char* dst, std::size_t size);
  Myt& get(std::uint8_t* dst, std::size_t size);

private:
  std::istream* is{nullptr};
};
}
