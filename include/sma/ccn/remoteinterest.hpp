#pragma once

#include <sma/nodeid.hpp>

#include <sma/chrono.hpp>

namespace sma
{
struct Interest;

struct RemoteInterest {
  using hops_type = std::uint16_t;

  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  RemoteInterest(hops_type hops);

  inline void touch();
  bool update(hops_type hops);

  template <typename Duration = std::chrono::milliseconds>
  Duration age()
  {
    return std::chrono::duration_cast<Duration>(clock::now() - last_seen);
  }

  template <typename Duration>
  bool older_than(Duration age)
  {
    return this->age<Duration>() >= age;
  }

  hops_type hops;
  time_point last_seen;
};
}
