#pragma once

#include <sma/ccn/contenttype.hpp>
#include <sma/network_hops.hpp>

namespace sma
{
struct Interest {
  TRIVIALLY_SERIALIZABLE(Interest, type, distance)

  ContentType type;
  network_hops distance;

  Interest(ContentType type, network_hops distance = 0)
    : type(type)
    , distance(distance)
  {
  }

  Interest(Interest&&) = default;
  Interest(Interest const&) = default;
  Interest& operator=(Interest&&) = default;
  Interest& operator=(Interest const&) = default;
};
}
