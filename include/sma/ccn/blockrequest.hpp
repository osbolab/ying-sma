#pragma once

#include <sma/util/hash.hpp>

#include <sma/util/serial.hpp>
#include <sma/util/vec2d.hpp>

#include <cstdint>


namespace sma
{
struct BlockFragmentRequest {
  TRIVIALLY_SERIALIZABLE(BlockFragmentRequest, offset, size)

  std::uint32_t offset;
  std::uint32_t size;
  Vec2d position;

  BlockFragmentRequest(std::uint32_t offset, std::uint32_t size)
    : offset(offset)
    , size(size)
  {
  }

  BlockFragmentRequest(BlockFragmentRequest&&) = default;
  BlockFragmentRequest(BlockFragmentRequest const&) = default;

  BlockFragmentRequest& operator=(BlockFragmentRequest&&) = default;
  BlockFragmentRequest& operator=(BlockFragmentRequest const&) = default;
};


struct BlockRequest {
  TRIVIALLY_SERIALIZABLE(BlockRequest, hash, index, fragments, position)

  Hash hash;
  std::uint16_t index;
  std::vector<BlockFragmentRequest> fragments;


  BlockRequest(Hash hash,
               std::uint16_t index,
               std::vector<BlockFragmentRequest> fragments,
               Vec2d position)
    : hash(hash)
    , index(index)
    , fragments(std::move(fragments))
    , position(position)
  {
  }

  BlockRequest(BlockRequest&&) = default;
  BlockRequest(BlockRequest const&) = default;

  BlockRequest& operator=(BlockRequest&&) = default;
  BlockRequest& operator=(BlockRequest const&) = default;
};
}
