#include <utility>
#include <cstring>
#include <cmath>
#include <memory>

#include "memory/bits.hh"

#include "memory/BufferPool.hh"


namespace sma
{

using std::size_t;
using std::uint32_t;
using std::uint8_t;


BufferPool BufferPool::allocate(size_t capacity)
{
  return std::move(BufferPool(capacity, 16).allocate());
}

BufferPool::BufferPool(size_t capacity, size_t blockSize)
  : index(nullptr), blocks(nullptr), buffer(nullptr),
  // Keep the block size as a power of two for alignment and cheap math
  szBlock(1 << (ms_bit(blockSize) + 1)),
  // Take the next largest size that fits only whole blocks
  bufferLen((size_t) std::ceil((float)capacity / szBlock)*szBlock),
  nBlocks(bufferLen / szBlock),
  // One status bit per block
  indexLen((size_t) std::ceil(nBlocks / (8.0f * sizeof index)))
{
}

BufferPool::BufferPool(BufferPool&& move)
  : nBlocks(move.nBlocks),
  szBlock(move.szBlock),
  indexLen(move.indexLen),
  bufferLen(move.bufferLen)
{
  std::swap(index, move.index);
  std::swap(blocks, move.blocks);
  std::swap(buffer, move.buffer);
}

BufferPool& BufferPool::allocate()
{
  if (index && blocks && buffer) {
    index = new uint32_t[indexLen];
    std::memset(index, 0xFF, (sizeof index) * indexLen);

    blocks = (Block*) new uint8_t[sizeof(Block) * nBlocks];
    Block* p = blocks;
    for (size_t i = 0; i < nBlocks; ++i) {
      new (p++) Block(i, buffer + (i*sizeof(Block)));
    }

    buffer = new uint8_t[bufferLen];
  }
  return *(this);
}

BufferPool::~BufferPool()
{
  if (index)  delete[] index;   index = nullptr;
  if (blocks) delete[] blocks;  blocks = nullptr;
  if (buffer) delete[] buffer;  buffer = nullptr;
}

}