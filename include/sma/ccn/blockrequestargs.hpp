#pragma once

#include <sma/nodeid.hpp>
#include <sma/util/hash.hpp>
#include <sma/ccn/blockindex.hpp>
#include <sma/util/vec2d.hpp>

#include <sma/util/serial.hpp>

#include <cstdlib>
#include <cstdint>
#include <chrono>


namespace sma
{
struct BlockRequestArgs {
  TRIVIALLY_SERIALIZABLE(BlockRequestArgs,
                         hash,
                         index,
                         utility,
                         ttl_ms,
                         requester,
                         requester_position)

  template <typename D>
  BlockRequestArgs(Hash hash,
                   BlockIndex index,
                   float utility,
                   D ttl,
                   NodeId requester,
                   Vec2d requester_position)
    : hash(hash)
    , index(index)
    , utility(utility)
    , ttl_ms(std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count())
    , requester(requester)
    , requester_position(requester_position)
  {
  }

  template <typename D>
  BlockRequestArgs(std::pair<Hash, BlockIndex> block,
                   float utility,
                   D ttl,
                   NodeId requester,
                   Vec2d requester_position)
    : BlockRequestArgs(block.first,
                       block.second,
                       utility,
                       ttl,
                       requester,
                       requester_position)
  {
  }

  template <typename D>
  D ttl() const
  {
    return std::chrono::duration_cast<D>(std::chrono::milliseconds(ttl_ms));
  }

  Hash hash;
  BlockIndex index;
  float utility;
  std::uint32_t ttl_ms;
  NodeId requester;
  Vec2d requester_position;
};
}
