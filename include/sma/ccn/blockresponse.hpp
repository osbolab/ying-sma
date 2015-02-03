#pragma once

#include <sma/util/hash.hpp>

#include <sma/util/serial.hpp>

#include <cstdint>
#include <cstring>


namespace sma
{
struct BlockFragmentResponse {
  std::uint32_t offset;
  std::uint32_t size;
  std::uint8_t* data;

  BlockFragmentResponse(std::uint32_t offset,
                        std::uint32_t size,
                        std::uint8_t const* src)
    : offset(offset)
    , size(size)
    , data(new std::uint8_t[size])
  {
    std::memcpy(data, src, size);
  }

  DESERIALIZING_CTOR(BlockFragmentResponse)
    : INIT_FIELDS(offset, size)
    , data(new std::uint8_t[size])
  {
    GET_BYTES(data, size);
  }

  SERIALIZER()
  {
    PUT_FIELDS(offset, size);
    PUT_BYTES(data, size);
  }

  BlockFragmentResponse(BlockFragmentResponse&& rhs)
    : offset(rhs.offset)
    , size(rhs.size)
    , data(rhs.data)
  {
    rhs.data = nullptr;
  }

  BlockFragmentResponse(BlockFragmentResponse const& rhs)
    : BlockFragmentResponse(rhs.offset, rhs.size, rhs.data)
  {
  }

  BlockFragmentResponse& operator=(BlockFragmentResponse&& rhs)
  {
    offset = rhs.offset;
    size = rhs.size;
    std::swap(data, rhs.data);
    return *this;
  }

  BlockFragmentResponse& operator=(BlockFragmentResponse const& rhs)
  {
    offset = rhs.offset;
    size = rhs.size;
    delete[] data;
    data = new std::uint8_t[size];
    std::memcpy(data, rhs.data, size);
    return *this;
  }

  ~BlockFragmentResponse() { delete[] data; }
};


struct BlockResponse {
  TRIVIALLY_SERIALIZABLE(
      BlockResponse, hash, index, size_kB, fragments)

  Hash hash;
  std::uint16_t index;
  std::uint8_t size_kB;
  std::vector<BlockFragmentResponse> fragments;


  BlockResponse(Hash hash,
                std::uint16_t index,
                std::uint8_t size_kB,
                std::vector<BlockFragmentResponse> fragments)
    : hash(hash)
    , index(index)
    , size_kB(size_kB)
    , fragments(std::move(fragments))
  {
  }

  BlockResponse(BlockResponse&&) = default;
  BlockResponse(BlockResponse const&) = default;

  BlockResponse& operator=(BlockResponse&&) = default;
  BlockResponse& operator=(BlockResponse const&) = default;
};
}
