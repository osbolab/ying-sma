#include <cassert>

#ifdef _DEBUG
#include <iostream>
#endif

#include "memory/pool_ptr.hh"


namespace sma
{

using std::uint8_t;
using std::size_t;


pool_ptr pool_ptr::null() { return pool_ptr(); }

pool_ptr::pool_ptr()
  : pool(nullptr), pBlocks(nullptr), nBlocks(0), szBlock(0)
{
}

pool_ptr::pool_ptr(BufferPool* pool, BufferPool::Block** pBlocks, size_t count)
  : pool(pool), 
  pBlocks(pBlocks), 
  nBlocks(count), 
  szBlock(pool->szBlock),
  minIdx(0), 
  maxIdx(szBlock)
{
#ifdef _DEBUG
  std::cout << "[pool_ptr::()] "  << nBlocks 
            << " blocks("         << static_cast<void*>(this->pBlocks) << ")" << std::endl;
#endif
}

pool_ptr::~pool_ptr()
{
  if (!pool) {
    assert(!pBlocks && "Memory leak: pool is null, but we have an array of block pointers");
    return;
  }

  while (minIdx > 0) {
    --pBlocks;
    minIdx -= szBlock;
  }

#ifdef _DEBUG
  std::cout << "[pool_ptr::~] deallocating " << nBlocks << " blocks" << std::endl;
#endif
  // The pool needs this array to release the pBlocks so we'll let her clean it up.
  pool->deallocate(pBlocks, nBlocks);

  pool = nullptr;
  pBlocks = nullptr;
}

uint8_t& pool_ptr::operator [](size_t index)
{
  if (index >= maxIdx) {
    assert(index < nBlocks * szBlock && "Buffer index out of range");
    while (index >= maxIdx) {
      ++pBlocks;
      maxIdx += szBlock;
    }
    minIdx = maxIdx - szBlock;
  } else if (index < minIdx) {
    while (index < minIdx) {
      --pBlocks;
      minIdx -= szBlock;
    }
    maxIdx = minIdx + szBlock;
  }

  return (**pBlocks)[index - minIdx];
}

const uint8_t& pool_ptr::operator [](size_t index) const
{
  int shift = 0;

  if (index >= maxIdx) {
    assert(index < nBlocks * szBlock && "Buffer index out of range");
    while (index >= maxIdx) {
      ++shift;
      index -= szBlock;
    }
  } else if (index < minIdx) {
    while (index < minIdx) {
      --shift;
      index += szBlock;
    }
  }

  return (**(pBlocks+shift))[index - minIdx];
}

void pool_ptr::move(pool_ptr& dst, pool_ptr& src)
{
  dst.pool    = src.pool;         src.pool = nullptr; 
  dst.pBlocks = src.pBlocks;      src.pBlocks = nullptr;
  dst.nBlocks = src.nBlocks;
  dst.szBlock = src.szBlock;
  dst.minIdx  = src.minIdx;
  dst.maxIdx  = src.maxIdx;

#ifdef _DEBUG
  std::cout << "[pool_ptr::move]" << std::endl;
#endif
}

pool_ptr::pool_ptr(pool_ptr&& m)
{
  move(*(this), m);
}

pool_ptr& pool_ptr::operator =(pool_ptr&& m)
{
  move(*(this), m);
  return *(this);
}

}