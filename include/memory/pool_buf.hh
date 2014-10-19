#pragma once

#include <iostream>
#include <cstdlib>
#include <string>

#include "log.hh"
#include "pow2_math.hh"

//#include "buffer_pool.hh"

namespace sma
{

template<typename T, std::size_t BlockSize>
class pool_buf
{
  static_assert(BlockSize > 0 && ((BlockSize & (BlockSize-1)) == 0),
                "Block size must be a power of two.");

  template<typename T, std::size_t BlockSize>
  friend class buffer_pool;

public:
  pool_buf(pool_buf&& move);
  ~pool_buf();

  pool_buf& operator =(pool_buf&& move);

  // Copy at most `count` bytes from `src` to this buffer.
  std::size_t fill_from(const T* src, std::size_t count);

  // Copy at most `count` bytes from this buffer into `dst`.
  std::size_t read_into(T* dst, std::size_t count);

  // Releases any part of the buffer not currently in use back to the pool, decreasing the capacity
  // of this buffer.
  std::size_t shrink();

  T& operator[](std::size_t index);
  const T& operator[](std::size_t index) const;

private:
  pool_buf(buffer_pool<T, BlockSize>* pool, T* const * blocks, std::size_t count);

  pool_buf(const pool_buf& copy) = delete;
  pool_buf& operator =(const pool_buf& copy) = delete;

  // She owns the blocks' memory.
  buffer_pool<T, BlockSize>* pool;
  // Array of pointers to blocks in the pool; we don't own this. 
  // Return it to buffer_pool::deallocate and she'll delete it.
  T* const *  blocks;
  // The number of block pointers in the array
  std::size_t nr_blocks;
  std::size_t capacity;
  std::size_t size;
};


template<typename T, std::size_t BlockSize>
pool_buf<T, BlockSize>::pool_buf(buffer_pool<T, BlockSize>* pool,
                               T* const* blocks, std::size_t count)
: pool(pool), blocks(blocks), nr_blocks(count), capacity(count * BlockSize), size(0)
{
  LOG_D("[pool_buf::()] block*[" << nr_blocks << "] ("
        << static_cast<const void*>(this->blocks) << ")");
}


template<typename T, std::size_t BlockSize>
pool_buf<T, BlockSize>::pool_buf(pool_buf<T, BlockSize>&& move)
  : pool(move.pool), blocks(move.blocks), nr_blocks(move.nr_blocks), 
  capacity(move.capacity), size(move.size)
{
  LOG_D("[pool_buf::(&&)] " << static_cast<void*>(&move));
  move.pool = nullptr;
  move.blocks = nullptr;
}


template<typename T, std::size_t BlockSize>
pool_buf<T, BlockSize>::~pool_buf()
{
  if (!pool) {
    assert(!blocks && "Leaked block pointers: pool is null");
    return;
  }
  LOG_D("[pool_buf::~]");
  pool->deallocate(blocks, nr_blocks);
}


template<typename T, std::size_t BlockSize>
pool_buf<T, BlockSize>&
pool_buf<T, BlockSize>::operator=(pool_buf<T, BlockSize>&& move)
{
  LOG_D("[pool_buf::=(&&)] " << static_cast<void*>(&move));
  std::swap(blocks, move.blocks);
  std::swap(pool, move.pool);
  nr_blocks = move.nr_blocks;
  capacity  = move.capacity;
  size      = move.size;
  return (*this);
}

template<typename T, std::size_t BlockSize>
std::size_t
pool_buf<T, BlockSize>::fill_from(const T* src, std::size_t count)
{
  if (count > capacity) count = capacity;
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
pool_buf<T, BlockSize>::read_into(T* dst, std::size_t count)
{
  if (count > capacity) count = capacity;
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
pool_buf<T, BlockSize>::operator[](std::size_t index)
{
  if (index > size) {
    if (index >= capacity) {
      std::cerr
        << "!!! Fatal error:\n!!!     Memory pool buffer index out of range ("
        << index << " >= " << capacity << ")" << std::endl;
      std::terminate();
    }
    size = index;
  }
  // Dividing the index by the block size gives us its block index
  // and the remainder is the offset within that block.
  return (*(blocks + (index >> pow2_math<BlockSize>::div)))[index & pow2_math<BlockSize>::mod];
}


template<typename T, std::size_t BlockSize>
const T&
pool_buf<T, BlockSize>::operator[](std::size_t index) const
{
  if (index >= capacity) {
    std::cerr
      << "!!! Fatal error:\n!!! Memory pool buffer index out of range ("
      << index << " >= " << capacity << ")" << std::endl;
    std::terminate();
  }
  // see T& operator[]
  return (*(blocks + (index >> pow2_math<BlockSize>::div)))[index & pow2_math<BlockSize>::mod];
}

}