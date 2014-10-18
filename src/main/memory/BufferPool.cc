#include <cassert>
#include <cmath>
#include <iostream>

#include "memory/bits.hh"

#include "memory/BufferPool.hh"


namespace sma
{

BufferPool BufferPool::allocate(std::size_t capacity)
{
}

BufferPool::BufferPool(std::size_t capacity, std::size_t blockSize)
  : index(nullptr), blocks(nullptr), buffer(nullptr),
  // Keep blocks sized as a power of two for alignment
  szBlock(1 << (ms_bit(blockSize) + 1)),
  // Capacity is at least one block
  bufferLen((capacity > szBlocks) ? Capacity : szBlocks),
  nBlocks((std::size_t) std::ceil((float)bufferLen / szBlock)),
  indexLen((std::size_t) std::ceil(nBlocks / (8.0f * sizeof index)))
{
  assert(BlockSize > 0);
}

BufferPool::~BufferPool()
{
  index = new std::uint32_t[indexLen];
  buffer = new std::uint8_t[nBlocks * szBlock];
}

BufferPool::~BufferPool()
{
  if (index)  delete[] index;   index = nullptr;
  if (blocks) delete[] blocks;  blocks = nullptr;
  if (buffer) delete[] buffer;  buffer = nullptr;
}


}