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
  TRIVIALLY_SERIALIZABLE(
      BlockRequestArgs, block, utility, ttl_ms, requester, requester_position)

  template <typename D>
  BlockRequestArgs(BlockRef block,
                   float utility,
                   D ttl,
                   NodeId requester,
                   Vec2d requester_position)
    : block(block)
    , utility(utility)
    , ttl_ms(std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count())
    , requester(requester)
    , requester_position(requester_position)
  {
  }

  template <typename D>
  D ttl() const
  {
    return std::chrono::duration_cast<D>(std::chrono::milliseconds(ttl_ms));
  }

  BlockRef block;
  float utility;
  std::uint32_t ttl_ms;
  NodeId requester;
  Vec2d requester_position;
};
}
