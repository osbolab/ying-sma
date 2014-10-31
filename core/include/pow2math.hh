#pragma once

#include <cstdlib>

namespace sma
{

template<std::size_t n>
struct Pow2Math {
  // m >> shr_to_div is equivalent to m / n.
  static const std::size_t shr_to_div = Pow2Math<(n >> 1)>::shr_to_div + 1;
  // m & and_to_mod is equivalent to m % n.
  static const std::size_t and_to_mod = (Pow2Math<(n >> 1)>::and_to_mod << 1) | 1;
};

template<>
struct Pow2Math<1> {
  static const std::size_t shr_to_div = 0;
  static const std::size_t and_to_mod = 0;
};

}