#pragma once

#include <cstdlib>
#include <cstdint>

#include "pooled_buf.hh"


namespace sma
{

using std::size_t;
using std::uint8_t;
using std::uint32_t;


class BufferPool
{ 
  friend class pooled_buf;

public:
  static BufferPool allocate(size_t capacity);
  BufferPool(BufferPool&& move);

  pooled_buf get(size_t size);

  ~BufferPool();

private:
  struct Block
  {
    Block(size_t index, uint8_t* head) : index(index), head(head) {}
    size_t   index;
    uint8_t* head;
  };

  BufferPool(size_t capacity, size_t blockSize);

  BufferPool(const BufferPool& copy) = delete;
  BufferPool& operator =(const BufferPool& copy) = delete;

  BufferPool& allocate();

  const size_t szBlock;
  const size_t nBlocks;
  const size_t indexLen;
  const size_t bufferLen;

  // One bit per block
  uint32_t*    index;
  Block*            blocks;
  uint8_t*     buffer;
};

}