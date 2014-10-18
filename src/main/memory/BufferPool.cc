#include <cassert>
#include <cmath>
#include <iostream>

#include "memory/bits.hh"

#include "memory/BufferPool.hh"


namespace sma
{

template<std::size_t szBlock>
BufferPool<szBlock> BufferPool<szBlock>::create(std::size_t capacity)
{
  return std::move(BufferPool<szBlock>(capacity).allocate());
}

template<std::size_t szBlock>
BufferPool<szBlock>::BufferPool(std::size_t capacity)
  : index(nullptr), blocks(nullptr), buffer(nullptr),
  // Take the next largest size that fits only whole blocks
  bufferLen((std::size_t) std::ceil((float)capacity / szBlock)*szBlock),
  nBlocks(bufferLen / szBlock),
  // One status bit per block
  indexLen((std::size_t) std::ceil(nBlocks / (8.0f * sizeof index)))
{
}

template<std::size_t szBlock>
BufferPool<szBlock>::BufferPool(BufferPool&& move)
  : nBlocks(move.nBlocks), 
  szBlock(move.szBlock), 
  indexLen(move.indexLen), 
  bufferLen(move.bufferLen)
{
  std::swap(index, move.index);
  std::swap(blocks, move.blocks);
  std::swap(buffer, move.buffer);
}

template<std::size_t szBlock>
BufferPool<szBlock>& BufferPool<szBlock>::allocate()
{
  if (index || blocks || buffer) return;

  index = new std::uint32_t[indexLen];
  buffer = new std::uint8_t[bufferLen];
  blocks = new Block[nBlocks];
}

template<std::size_t szBlock>
BufferPool<szBlock>::~BufferPool()
{
  if (index)  delete[] index;   index = nullptr;
  if (blocks) delete[] blocks;  blocks = nullptr;
  if (buffer) delete[] buffer;  buffer = nullptr;
}

}