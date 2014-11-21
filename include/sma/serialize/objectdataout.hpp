#pragma once

#include <cstdint>
#include <utility>

namespace sma
{
template <typename Formatter>
class ObjectDataOut final
{
  using Myt = ObjectDataOut<Formatter>;

public:
  ObjectDataOut(Formatter formatter)
    : f(std::move(formatter))
  {
  }

  // clang-format off
  template <typename T> Myt& put(T t) { f.put(std::forward<T>(t)); return *this; }
  Myt& put(std::int8_t const* src, std::size_t size) { f.put(src, size); return *this; }
  Myt& put(std::uint8_t const* src, std::size_t size) { f.put(src, size); return *this; }
  // clang-format on

private:
  Formatter f;
};
}
