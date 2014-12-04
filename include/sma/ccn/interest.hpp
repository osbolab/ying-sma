#pragma once

#include <sma/ccn/contenttype.hpp>
#include <sma/nodeid.hpp>

namespace sma
{
struct Interest {
  TRIVIALLY_SERIALIZABLE(Interest, type)

  using dirty_neighbor_set = std::vector<NodeId>;

  //! The type of content in which interest was shown.
  ContentType type;
  //! The dirty neighbor set names any neighbors we have *not* seen rebroadcast
  //! this interest.
  dirty_neighbor_set dirty_neighbors;

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
