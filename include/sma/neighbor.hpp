#pragma once

#include <sma/chrono.hpp>

namespace sma
{
struct Neighbor {
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  Neighbor()
    : first_seen(clock::now())
  {
    saw();
  }

  bool is_new() const { return first_seen == last_seen; }

  template <typename D>
  bool older_than(D const& age) const
  {
    return std::chrono::duration_cast<D>(clock::now() - last_seen) >= age;
  }

  void saw()
  {
    last_seen = clock::now();
    times_pinged = 0;
  }

  time_point first_seen;
  time_point last_seen;
  unsigned int times_pinged;
};
}
