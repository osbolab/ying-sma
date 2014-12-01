#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <iosfwd>

namespace sma
{

class BinaryInput
{
  std::istream* is;

public:
  BinaryInput(std::istream& is)
    : is(&is)
  {
  }

  BinaryInput(BinaryInput const& r)
    : is(r.is)
  {
  }

  BinaryInput& operator=(BinaryInput const& r)
  {
    is = r.is;
    return *this;
  }

  void* read(void* dst, std::size_t size);

  template <typename T>
  T get();
};

// clang-format off
template <> std::string   BinaryInput::get<std::string>();
template <> float         BinaryInput::get<float>();
template <> double        BinaryInput::get<double>();
template <> std::uint8_t  BinaryInput::get<std::uint8_t>();
template <> std::uint16_t BinaryInput::get<std::uint16_t>();
template <> std::uint32_t BinaryInput::get<std::uint32_t>();
template <> std::uint64_t BinaryInput::get<std::uint64_t>();
// clang-format on
}
