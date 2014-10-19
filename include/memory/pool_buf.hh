#pragma once

#include <cstdlib>

//#include "BufferPool.hh"


namespace sma
{

template<class T, std::size_t szBlock>
class pool_buf
{
  friend class BufferPool;

public:
  pool_buf(pool_buf&& move);
  pool_buf& operator =(pool_buf&& move);

  ~pool_buf();

  //! Copy at most `count` bytes from buf to this buffer.
  void fill(const T* buf, std::size_t count);
  std::size_t read(T* buf, std::size_t count);

  //! Releases to the pool memory that isn't being used by the contents of this buffer.
  std::size_t shrink();

  T& operator [](std::size_t index);
  const T& operator [](std::size_t index) const;

private:
  pool_buf();
  pool_buf(std::shared_ptr<BufferPool<T, szBlock>> pool, T* const * pBlocks, std::size_t count);

  pool_buf(const pool_buf& copy) = delete;
  pool_buf& operator =(const pool_buf& copy) = delete;

  static void move(pool_buf& dst, pool_buf& src);

  // She owns the blocks' memory.
  std::shared_ptr<BufferPool<T, szBlock>> pool;
  // The number of block pointers in the array
  std::size_t nBlocks;
  // Array of pointers to blocks in the pool; we don't own this. 
  // Return it to BufferPool::deallocate and she'll delete it.
  T* const *  pBlocks;
  // Starts out as nBlocks * szBlock, but if the buffer is resized then this
  // reflects the size of the data that has been placed in it.
  std::size_t size;
};


template<std::size_t n>
struct indexMath
{
  // The number of bits by which shifting an index will determine which block it's in.
  // Equivalent to (index / szBlock) when szBlock = 2^n
  static const std::size_t divisor = indexMath<(n >> 1)>::divisor + 1;
  // The bitwise mask that produces the remainder of (index / szBlock).
  // Equivalent to (index % szBlock) when szBlock = 2^n
  static const std::size_t mask = (indexMath<(n >> 1)>::mask << 1) | 1;
};
template<>
struct indexMath<1>
{
  static const std::size_t divisor = 0;
  static const std::size_t mask = 0;
};


template<class T, std::size_t szBlock>
pool_buf<T, std::size_t szBlock>::pool_buf(BufferPool<T, szBlock>* pool, 
                                           T* const* pBlocks, 
                                           std::size_t count)
  : pool(pool), pBlocks(pBlocks), nBlocks(count), size(nBlocks * szBlock)
{
#ifdef _DEBUG
  std::cout << "[pool_buf::()] "  << nBlocks
    << " blocks("         << static_cast<const void*>(this->pBlocks) << ")" << std::endl;
#endif
}


template<class T>
pool_buf<T>::~pool_buf()
{
  if (!pool) {
    assert(!pBlocks && "Memory leak: pool is null, but we have an array of block pointers");
    return;
  }

#ifdef _DEBUG
  std::cout << "[pool_buf::~] deallocating " << nBlocks << " blocks" << std::endl;
#endif

  pool->deallocate(pBlocks, nBlocks);

  pool = nullptr;
  pBlocks = nullptr;
}


template<class T, std::size_t szBlock>
void pool_buf<T, szBlock>::fill(const T* buf, std::size_t count)
{
  if (count > size) count = size;

  T* const* block = pBlocks;
  while (count > szBlock) {
    std::memcpy(*(block++), buf, szBlock);
    count -= szBlock;
    buf += szBlock;
  }
  std::memcpy(*block, buf, count);
}


template<class T, std::size_t szBlock>
std::size_t pool_buf<T, szBlock>::read(T* buf, std::size_t count)
{
  if (count > size) count = size;

  T* const* block = pBlocks;
  while (count > szBlock) {
    std::memcpy(buf, *(block++), szBlock);
    count -= szBlock;
    buf += szBlock;
  }
  std::memcpy(buf, *(block++), count);

  return size;
}


template<class T, std::size_t szBlock>
T& pool_buf<T, szBlock>::operator [](std::size_t index)
{
  // Dividing the index by the block size gives us the block its on
  // and the remainder is the index within that block.
  return (*(pBlocks + (index >> indexMath<szBlock>::divisor)))[index & indexMath<szBlock>::mask];
}


template<class T, std::size_t szBlock>
const T& pool_buf<T, szBlock>::operator [](std::size_t index) const
{
  return (*(pBlocks + (index >> indexMath<szBlock>::divisor)))[index & indexMath<szBlock>::mask];
}


template<class T, std::size_t szBlock>
void pool_buf<T, szBlock>::move(pool_buf<T, szBlock>& dst, pool_buf<T, szBlock>& src)
{
  dst.pool    = src.pool;         src.pool    = nullptr;
  dst.pBlocks = src.pBlocks;      src.pBlocks = nullptr;
  dst.nBlocks = src.nBlocks;
}


template<class T, std::size_t szBlock>
pool_buf<T, szBlock>::pool_buf(pool_buf<T, szBlock>&& m)
{
  move((*this), m);
}


template<class T, std::size_t szBlock>
pool_buf<T, szBlock>& pool_buf<T, szBlock>::operator =(pool_buf<T, szBlock>&& m)
{
  move((*this), m);
  return (*this);
}

}