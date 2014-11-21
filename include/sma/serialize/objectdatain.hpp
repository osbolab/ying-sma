#pragma once

#include <cstdint>


namespace sma
{
template <typename Formatter>
class ObjectDataIn final
{
  using Myt = ObjectDataIn<Formatter>;

public:
  ObjectDataIn(Formatter formatter)
    : f(std::move(formatter))
  {
  }

  // clang-format off
  template <typename T> T get() { return f.get<T>(); }
  Myt& get(std::int8_t* dst, std::size_t size) { f.get(dst, size); return *this; }
  Myt& get(std::uint8_t* dst, std::size_t size) { f.get(dst, size); return *this; }
  // clang-format on

private:
  Formatter f;
};
}
