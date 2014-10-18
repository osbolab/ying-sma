#pragma once

#include <cstdlib>
#include <cstdint>

#include "pooled_buf.hh"


namespace sma
{

template<std::size_t szBlock>
class BufferPool
{
  static_assert((szBlock > 1) && (szBlock &(szBlock - 1)) == 0,
                "Block size must be a power of two.");

  friend class pooled_buf;

public:
  static BufferPool<szBlock> create(std::size_t capacity);
  BufferPool(BufferPool<szBlock>&& move);

  ~BufferPool();

private:
  template<std::size_t szBlock>
  struct Block
  {
    Block(std::size_t index, std::uint8_t* head) : index(index), head(head) {}
    std::size_t   index;
    std::uint8_t* head;
  };

  BufferPool(std::size_t capacity);

  BufferPool(const BufferPool& copy) = delete;
  BufferPool& operator =(const BufferPool& copy) = delete;

  BufferPool& allocate();

  const std::size_t nBlocks;
  const std::size_t indexLen;
  const std::size_t bufferLen;

  // One bit per block
  std::uint32_t*    index;
  Block<szBlock>*   blocks;
  std::uint8_t*     buffer;
};

}