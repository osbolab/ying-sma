#pragma once

#include <cstdlib>

namespace sma
{

template<std::size_t n>
struct Pow2Math {
  // m >> div is equivalent to m / n.
  static const std::size_t div = Pow2Math<(n >> 1)>::div + 1;
  // m & mod is equivalent to m % n.
  static const std::size_t mod = (Pow2Math<(n >> 1)>::mod << 1) | 1;
};

template<>
struct Pow2Math<1> {
  static const std::size_t div = 0;
  static const std::size_t mod = 0;
};

}