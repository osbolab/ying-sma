#include <cassert>

#ifdef _DEBUG
#include <iostream>
#endif

#include "memory/pool_buf.hh"


namespace sma
{

using std::uint8_t;
using std::size_t;


pool_buf::pool_buf(BufferPool* pool, std::uint8_t* const* pBlocks, size_t count)
  : pool(pool),
  pBlocks(pBlocks),
  nBlocks(count),
  szBlock(pool->szBlock),
  indexDivisor(0),
  indexMask(0)
{
  // Determine the number of shifts needed to divide something by szBlock
  // and the mask that gives us the remainder.
  for (size_t sz = szBlock; sz >= 1; sz >>= 1) {
    ++indexDivisor;
    indexMask = (indexMask << 1) | 1;
  }
  
#ifdef _DEBUG
  std::cout << "[pool_buf::()] "  << nBlocks 
            << " blocks("         << static_cast<const void*>(this->pBlocks) << ")" << std::endl;
#endif
}

pool_buf::~pool_buf()
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

void pool_buf::fill(const uint8_t* buf, size_t count)
{
  if (count > nBlocks*szBlock) count = nBlocks*szBlock;
  
  std::uint8_t* const* block = pBlocks;
  while (count > 0) {
    for (size_t i = 0; i < szBlock; ++i) {
      (*block)[i] = *(buf++);
    }
    ++block;
    count -= szBlock;
  }
}

size_t pool_buf::read(std::uint8_t* buf, std::size_t count)
{
  return 0;
}

uint8_t& pool_buf::operator [](size_t index)
{
  // Dividing the index by the block size gives us the block its on
  // and the remainder is the index within that block.
  return (**(pBlocks + (index >> indexDivisor)))[index & indexMask];
}

const uint8_t& pool_buf::operator [](size_t index) const
{
  return (**(pBlocks + (index >> indexDivisor)))[index & indexMask];
}

void pool_buf::move(pool_buf& dst, pool_buf& src)
{
  dst.pool    = src.pool;         src.pool    = nullptr; 
  dst.pBlocks = src.pBlocks;      src.pBlocks = nullptr;
  dst.nBlocks = src.nBlocks;
  dst.szBlock = src.szBlock;
  dst.indexDivisor = src.indexDivisor;
  dst.indexMask = src.indexMask;
}

pool_buf::pool_buf(pool_buf&& m)
{
  move((*this), m);
}

pool_buf& pool_buf::operator =(pool_buf&& m)
{
  move((*this), m);
  return (*this);
}

}