#pragma once

#include <cstdlib>

namespace sma
{

template<std::size_t n>
struct pow2_math
{
  // m >> div is equivalent to m / n.
  static const std::size_t div = pow2_math<(n >> 1)>::div + 1;
  // m & mod is equivalent to m % n.
  static const std::size_t mod = (pow2_math<(n >> 1)>::mod << 1) | 1;
};

template<>
struct pow2_math<1>
{
  static const std::size_t div = 0;
  static const std::size_t mod = 0;
};

}