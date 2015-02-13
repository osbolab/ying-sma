#pragma once

#include <sma/util/vec2d.hpp>
#include <sma/util/hash.hpp>
#include <sma/ccn/blockref.hpp>
#include <ctime>
#include <sma/chrono.hpp>
#include <sma/nodeid.hpp>
#include <sma/ccn/ccnnode.hpp>

namespace sma
{
struct BlockRequestDesc
{
  using timestamp = sma::chrono::system_clock::time_point;
  Hash content_name;
  BlockIndex block_index;
  float utility;
  timestamp expire_time;
  NodeId from;
  NodeId requester;
  Vec2d origin_location;

  BlockRequestDesc (Hash name,
                    BlockIndex index,
                    float util,
                    timestamp expire,
                    NodeId neighbor_id,
                    NodeId requester_node,
                    Vec2d location)
      : content_name (name)
      , block_index (index)
      , utility (util)
      , expire_time (expire)
      , from (neighbor_id)
      , requester (requester_node)
      , origin_location (location)
  {}
};

}
