#pragma once

#include <chrono>


namespace sma
{

constexpr std::chrono::milliseconds operator _ms(long long n)
{
  return std::chrono::milliseconds { n };
}

}