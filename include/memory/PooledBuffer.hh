#pragma once

#include <iostream>
#include <cstdlib>
#include <string>

#include "Log.hh"
#include "Pow2Math.hh"

// #define RANGE_CHECKED_BUFFERS_

namespace sma
{

template<typename T, std::size_t BlockSize>
class PooledBuffer
{
  static_assert(BlockSize > 0 && ((BlockSize & (BlockSize-1)) == 0),
                "Block size must be a power of two.");

  template<typename T, std::size_t BlockSize>
  friend class BufferPool;

public:
  PooledBuffer(PooledBuffer&& move);
  ~PooledBuffer();

  PooledBuffer& operator =(PooledBuffer&& move);

  // Copy at most `count` bytes from `src` to this buffer.
  std::size_t fill_with(const T* src, std::size_t count);

  // Copy at most `count` bytes from this buffer into `dst`.
  std::size_t read_into(T* dst, std::size_t count);

  /*! Releases any part of the buffer not currently in use back to the pool, decreasing the capacity
   * of this buffer.
   * \return the buffer's new capacity.
   */
  std::size_t shrink_to_fit();

  T& operator[](std::size_t index);
  const T& operator[](std::size_t index) const;

  std::size_t get_size() const { return size; }
  std::size_t get_capacity() const { return capacity; }

private:
  PooledBuffer(BufferPool<T, BlockSize>* pool, T* const* blocks, std::size_t count);

  PooledBuffer(const PooledBuffer& copy) = delete;
  PooledBuffer& operator =(const PooledBuffer& copy) = delete;

  // She owns the blocks' memory.
  BufferPool<T, BlockSize>* pool;
  // Array of pointers to blocks in the pool; we don't own this.
  // Return it to BufferPool::deallocate and she'll delete it.
  T* const*   blocks;
  // The number of block pointers in the array
  std::size_t nr_blocks;
  std::size_t capacity;
  std::size_t size;
};


template<typename T, std::size_t BlockSize>
PooledBuffer<T, BlockSize>::PooledBuffer(BufferPool<T, BlockSize>* pool,
    T* const* blocks, std::size_t count)
  : pool(pool), blocks(blocks), nr_blocks(count), capacity(count* BlockSize), size(0)
{
  LOG_D("[PooledBuffer::()] block*[" << nr_blocks << "] ("
        << static_cast<const void*>(this->blocks) << ")");
}


template<typename T, std::size_t BlockSize>
PooledBuffer<T, BlockSize>::PooledBuffer(PooledBuffer<T, BlockSize>&& move)
  : pool(move.pool), blocks(move.blocks), nr_blocks(move.nr_blocks),
    capacity(move.capacity), size(move.size)
{
  LOG_D("[PooledBuffer::(&&)] " << static_cast<void*>(&move));
  move.pool = nullptr;
  move.blocks = nullptr;
}


template<typename T, std::size_t BlockSize>
PooledBuffer<T, BlockSize>::~PooledBuffer()
{
  if (!pool) {
    assert(!blocks && "Leaked block pointers: pool is null");
    return;
  }
  LOG_D("[PooledBuffer::~]");
  pool->deallocate(blocks, nr_blocks);
}


template<typename T, std::size_t BlockSize>
PooledBuffer<T, BlockSize>&
PooledBuffer<T, BlockSize>::operator=(PooledBuffer<T, BlockSize>&& move)
{
  LOG_D("[PooledBuffer::=(&&)] " << static_cast<void*>(&move));
  std::swap(blocks, move.blocks);
  std::swap(pool, move.pool);
  nr_blocks = move.nr_blocks;
  capacity  = move.capacity;
  size      = move.size;
  return (*this);
}


template<typename T, std::size_t BlockSize>
std::size_t
PooledBuffer<T, BlockSize>::shrink_to_fit()
{
  std::size_t nr_unused_blocks = (capacity - size) >> Pow2Math<BlockSize>::div;
  LOG_D("[PooledBuffer::shrink_to_fit] " << nr_unused_blocks << " unused blocks");
  if (!nr_unused_blocks) return capacity;
  LOG_D("[PooledBuffer::shrink_to_fit] shrink from " << capacity << " bytes");
  nr_blocks -= nr_unused_blocks;
  capacity = nr_blocks * BlockSize;
  T* const* unused_blocks = blocks + nr_blocks;
  pool->deallocate(unused_blocks, nr_unused_blocks);
  LOG_D("[PooledBuffer::shrink_to_fit] shrunk to " << capacity << " bytes");
  return capacity;
}


template<typename T, std::size_t BlockSize>
std::size_t
PooledBuffer<T, BlockSize>::fill_with(const T* src, std::size_t count)
{
  if (count > capacity) { count = capacity; }
  T* const* block = blocks;
  const size_t nr_read = count;

  while (count > BlockSize) {
    std::memcpy(*block++, src, BlockSize);
    count -= BlockSize;
    src += BlockSize;
  }
  std::memcpy(*block, src, count);

  size = nr_read;
  return nr_read;
}


template<typename T, std::size_t BlockSize>
std::size_t
PooledBuffer<T, BlockSize>::read_into(T* dst, std::size_t count)
{
  if (count > capacity) { count = capacity; }
  T* const* block = blocks;
  const size_t nr_read = count;

  while (count > BlockSize) {
    std::memcpy(dst, *block++, BlockSize);
    count -= BlockSize;
    dst += BlockSize;
  }
  std::memcpy(dst, *block, count);

  return nr_read;
}


template<typename T, std::size_t BlockSize>
T&
PooledBuffer<T, BlockSize>::operator[](std::size_t index)
{
  if (index > size) {
#ifdef RANGE_CHECKED_BUFFERS_
    if (index >= capacity) {
      std::cerr
          << "!!! Fatal error:\n!!!     Memory pool buffer index out of range ("
          << index << " >= " << capacity << ")" << std::endl;
      std::terminate();
    }
#endif
    size = index;
  }
  // Dividing the index by the block size gives us its block index
  // and the remainder is the offset within that block.
  return (*(blocks + (index >> Pow2Math<BlockSize>::div)))[index & Pow2Math<BlockSize>::mod];
}


template<typename T, std::size_t BlockSize>
const T&
PooledBuffer<T, BlockSize>::operator[](std::size_t index) const
{
#ifdef RANGE_CHECKED_BUFFERS_
  if (index >= capacity) {
    std::cerr
        << "!!! Fatal error:\n!!! Memory pool buffer index out of range ("
        << index << " >= " << capacity << ")" << std::endl;
    std::terminate();
  }
#endif
  // see T& operator[]
  return (*(blocks + (index >> Pow2Math<BlockSize>::div)))[index & Pow2Math<BlockSize>::mod];
}

}