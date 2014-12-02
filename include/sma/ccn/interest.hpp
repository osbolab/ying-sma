#pragma once

#include <sma/ccn/contenttype.hpp>

namespace sma
{
struct Interest {
  TRIVIALLY_SERIALIZABLE(Interest, type)

  ContentType type;

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
