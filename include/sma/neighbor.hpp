#pragma once

#include <sma/chrono>

#include <sma/nodeid.hpp>

namespace sma
{
  struct Neighbor
  {
    using time_point = sma::chrono::system_clock::time_point;

    bool is_new() const { return first_seen == last_seen; }

    NodeId node;
    time_point first_seen;
    time_point last_seen;
  };
}
