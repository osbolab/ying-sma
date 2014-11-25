#pragma once

#include <sma/chrono>

#include <sma/nodeid.hpp>

namespace sma
{
struct Neighbor {
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  Neighbor(NodeId node)
    : node(node)
  {
    touch();
  }

  bool is_new() const { return first_seen == last_seen; }

  template <typename D>
  bool expired(D max_age) const
  {
    return std::chrono::duration_cast<D>(clock::now() - last_seen) >= max_age;
  }

  void touch() { last_seen = clock::now(); }

  NodeId node;
  time_point first_seen;
  time_point last_seen;
};
}
