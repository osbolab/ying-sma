#pragma once

#include <sma/nodeid.hpp>
#include <sma/network_hops.hpp>

#include <sma/chrono.hpp>

namespace sma
{
struct Interest;

struct RemoteInterest {
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  RemoteInterest(Interest const& interest);

  inline void touch();
  bool update(Interest const& interest);

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

  network_hops distance;
  time_point last_seen;
};
}
