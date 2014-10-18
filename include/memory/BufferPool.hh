#pragma once

#include <cstdlib>
#include <cstdint>

#include "pooled_buf.hh"


namespace sma
{

class BufferPool
{
  friend class pooled_buf;

public:
  static BufferPool allocate(std::size_t capacity);

  ~BufferPool();

private:
  struct Block
  {
  };

  BufferPool(std::size_t capacity, std::size_t blockSize);

  const std::size_t nBlocks;
  const std::size_t szBlock;
  const std::size_t indexLen;
  const std::size_t bufferLen;

  // One bit per block
  std::uint32_t*    index;
  Block*            blocks;
  std::uint8_t*     buffer;
};

}