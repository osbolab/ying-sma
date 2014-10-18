#pragma once

#include <cstdlib>

#include "BufferPool.hh"


namespace sma
{

class pool_ptr
{
  friend class BufferPool;

public:
  pool_ptr(pool_ptr&& move);
  pool_ptr& operator =(pool_ptr&& move);

  ~pool_ptr();

  std::uint8_t& operator [](std::size_t index);
  const std::uint8_t& operator [](std::size_t index) const;

  static pool_ptr null();

private:
  pool_ptr();
  pool_ptr(BufferPool* pool, BufferPool::Block** blocks, std::size_t count);

  pool_ptr(const pool_ptr& copy) = delete;
  pool_ptr& operator =(const pool_ptr& copy) = delete;

  static void move(pool_ptr& dst, pool_ptr& src);

  // She owns the blocks' memory
  BufferPool*         pool;
  // Array of pointers to blocks in the pool; we don't own this. 
  // Return it to BufferPool::deallocate and she'll delete the array.
  BufferPool::Block** pBlocks;
  std::size_t         nBlocks;
  std::size_t         szBlock;
  // Track our view into the blocks array in terms of overall buffer indices
  std::size_t         minIdx;
  std::size_t         maxIdx;
};

}