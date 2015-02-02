#pragma once

#include <sma/util/hash.hpp>

#include <sma/util/serial.hpp>

#include <cstdint>


namespace sma
{
struct BlockFragmentRequest {
  TRIVIALLY_SERIALIZABLE(BlockFragmentRequest, offset, size)

  std::uint32_t offset;
  std::uint32_t size;

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
  TRIVIALLY_SERIALIZABLE(BlockRequest, content_hash, block_idx, fragments)

  Hash content_hash;
  std::uint16_t block_idx;
  std::vector<BlockFragmentRequest> fragments;


  BlockRequest(Hash content_hash,
               std::uint16_t block_idx,
               std::vector<BlockFragmentRequest> fragments)
    : content_hash(content_hash)
    , block_idx(block_idx)
    , fragments(std::move(fragments))
  {
  }

  BlockRequest(BlockRequest&&) = default;
  BlockRequest(BlockRequest const&) = default;

  BlockRequest& operator=(BlockRequest&&) = default;
  BlockRequest& operator=(BlockRequest const&) = default;
};
}
