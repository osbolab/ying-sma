#pragma once

#include <sma/util/hash.hpp>
#include <sma/util/vec2d.hpp>

#include <sma/util/serial.hpp>

#include <cstdlib>
#include <cstdint>
#include <chrono>


namespace sma
{
struct BlockRequestArgs {
  TRIVIALLY_SERIALIZABLE(BlockRequestArgs, hash, index, utility, ttl_ms, origin)

  template <typename D>
  BlockRequestArgs(
      Hash hash, std::size_t index, double utility, D ttl, Vec2d origin)
    : hash(hash)
    , index(index)
    , utility(utility)
    , ttl_ms(std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count())
    , origin(origin)
  {
  }

  template <typename D>
  BlockRequestArgs(std::pair<Hash, std::size_t> block,
                   double utility,
                   D ttl,
                   Vec2d origin)
    : BlockRequestArgs(block.first, block.second, utility, ttl, origin)
  {
  }

  template <typename D>
  D ttl() const
  {
    return std::chrono::duration_cast<D>(std::chrono::milliseconds(ttl_ms));
  }

  Hash hash;
  std::uint32_t index;
  double utility;
  std::uint32_t ttl_ms;
  Vec2d origin;
};
}
