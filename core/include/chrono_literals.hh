#pragma once

#include <chrono>


namespace sma
{

static constexpr std::chrono::milliseconds operator"" _ms(unsigned long long n)
{
  return std::chrono::milliseconds{n};
}

template <typename T>
static auto to_millis(T const& t)
    -> decltype(std::chrono::duration_cast<std::chrono::milliseconds>(t))
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(t);
}
}
