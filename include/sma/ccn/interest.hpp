#pragma once

#include <sma/ccn/contenttype.hpp>

namespace sma
{
struct Interest {
  TRIVIALLY_SERIALIZABLE(Interest, type, hops)

  using hops_type = std::uint8_t;

  ContentType type;
  hops_type hops = 0;

  Interest(ContentType type)
    : type(type)
  {
  }

  Interest(Interest&&) = default;
  Interest(Interest const&) = default;
Interest& operator=(Interest&&) = default;
Interest& operator=(Interest const&) = default;
};
}
