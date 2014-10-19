#pragma once

#include <cstdlib>

template<std::size_t width>
struct indexMath__
{
  static const std::size_t divisor = indexMath__<(width >> 1)>::divisor + 1;
  static const std::size_t mask = (indexMath__<(width >> 1)>::mask << 1) | 1;
};

template<>
struct indexMath__<1>
{
  static const std::size_t divisor = 0;
  static const std::size_t mask = 0;
};

template<class T, std::size_t sz>
struct IndexMath
{
  static const int divisor = indexMath__<sz>::divisor;
  static const int mask = indexMath__<sz>::mask;
};