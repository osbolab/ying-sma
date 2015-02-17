#include <sma/chrono.hpp>

#include <chrono>


namespace sma
{
chrono::system_clock::time_point chrono::system_clock::now()
{
  using clock = chrono::system_clock;
  using sys = std::chrono::system_clock;

  return clock::from_time_t(sys::to_time_t(sys::now()));
}
}
