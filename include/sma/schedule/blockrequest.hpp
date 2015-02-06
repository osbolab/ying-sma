#pragma once

#include <sma/util/hash.hpp>
#include <sma/nodeid.hpp>

namespace sma
{
class BlockRequest
{
public:

  BlockRequest (Hash content_name, std::size seq, NodeID id)
      : name(content_name)
      , index (seq)
      , node_id (id)
      , origin (location)
  {}

  Hash get_name() const;
  std::size_t get_index() const;
  NodeID get_node_id_from() const;

private:
  Hash name;
  std::size_t index;
  NodeID node_id;
};
}
