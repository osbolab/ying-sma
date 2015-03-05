#pragma once

#include <sma/nodeid.hpp>
#include <sma/ccn/blockref.hpp>

#include <sma/util/hash.hpp>
#include <sma/util/vec2d.hpp>
#include <sma/util/serial.hpp>

#include <cstdlib>
#include <cstdint>
#include <chrono>


namespace sma
{
struct BlockRequestArgs {
  TRIVIALLY_SERIALIZABLE(BlockRequestArgs,
                         block,
                         utility,
                         ttl_ms,
                         requester,
                         requester_position,
                         hops_from_block)

  template <typename D>
  BlockRequestArgs(BlockRef block,
                   float utility,
                   D ttl,
                   NodeId requester,
                   Vec2d requester_position,
                   std::uint8_t hops_from_block,
                   bool keep_on_arrival = false,
                   bool local_only = false)
    : block(block)
    , utility(utility)
    , ttl_ms(std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count())
    , requester(requester)
    , requester_position(requester_position)
    , hops_from_block(hops_from_block)
    , keep_on_arrival(keep_on_arrival)
    , local_only(local_only)
  {
  }

  template <typename D>
  D ttl() const
  {
    return std::chrono::duration_cast<D>(std::chrono::milliseconds(ttl_ms));
  }

  // Serialized

  BlockRef block;
  float utility;
  std::uint32_t ttl_ms;
  NodeId requester;
  Vec2d requester_position;
  std::uint8_t hops_from_block;

  // Transient

  // Signal to the content cache that this block should be stored permanently.
  bool keep_on_arrival;
  // Signal that this should not be forwarded
  bool local_only;
};
}
