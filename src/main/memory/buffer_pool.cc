#include <utility>
#include <cstring>
#include <cmath>
#include <memory>
#include <iostream>
#include <cassert>

#include "memory/bits.hh"

#define MT_POOL_

#include "memory/pool_buf.hh"
#include "memory/BufferPool.hh"


namespace sma
{

using std::size_t;
using std::uint32_t;
using std::uint8_t;
using std::unique_ptr;
#ifdef MT_POOL_
using std::mutex;
using std::unique_lock;
#endif




/*********************************************
 * Static factory methods
 */

BufferPool BufferPool::create(size_t capacity)
{
  auto pool = BufferPool(capacity, 16);
  pool.initialize();
  return pool;
}

/** Static factory methods
 *********************************************/


/*********************************************
 * Construction/Initialization/Destruction
 */

BufferPool::BufferPool(size_t capacity, size_t blockSize)
  : buffer(nullptr), blocks(nullptr), statusBitmap(nullptr), numBlocksAvailable(0)
{
  // Ensure the block size is a power of two for alignment and fast math
  szBlock = ((blockSize & (blockSize-1)) == 0) ? blockSize : 1 << (most_set_bit(blockSize) + 1);
  // Increase the capacity to fit only whole blocks
  capacity = szBlock * (size_t) std::ceil((float) capacity / szBlock);
  nBlocks = capacity / szBlock;

#ifdef _DEBUG
  std::cout << "[BufferPool::()] " << nBlocks << " blocks of " << szBlock << " bytes";
#ifndef MT_POOL_
  std::cout                         << " (NOT thread safe)";
#endif
  std::cout << std::endl;
#endif
}

BufferPool::BufferPool(BufferPool&& move)
  : buffer(nullptr), blocks(nullptr), statusBitmap(nullptr)
{
  szBlock = move.szBlock;
  nBlocks = move.nBlocks;
  numBlocksAvailable = move.numBlocksAvailable;

  std::swap(buffer, move.buffer);
  std::swap(blocks, move.blocks);
  std::swap(statusBitmap, move.statusBitmap);
}

BufferPool& BufferPool::operator =(BufferPool&& move)
{
  szBlock = move.szBlock;
  nBlocks = move.nBlocks;
  numBlocksAvailable = move.numBlocksAvailable;

  std::swap(buffer, move.buffer);
  std::swap(blocks, move.blocks);
  std::swap(statusBitmap, move.statusBitmap);

  return *this;
}

void BufferPool::initialize()
{
  if (statusBitmap || blocks || buffer) return;

  buffer = new uint8_t[nBlocks * szBlock];

  // One status bit per block
  const size_t bitmapLen = (size_t) std::ceil((float) nBlocks / status_t_size_bits);
  // Set the default status of every block to 'available'
  statusBitmap = new status_t[bitmapLen];
  std::memset(statusBitmap, 0xFF, sizeof(status_t) * bitmapLen);

  // Construct and initialize blocks in place
  blocks = (Block*) new uint8_t[sizeof(Block) * nBlocks];
  Block* p = blocks;
  for (size_t i = 0; i < nBlocks; ++i) {
    new (p++) Block(i, buffer + (i * sizeof(Block)), szBlock);
  }
  numBlocksAvailable = nBlocks;

#ifdef _DEBUG
  std::cout
    << "[BufferPool::init] buffer(" << static_cast<void*>(buffer) << "+" << (nBlocks * szBlock)
    << "), blocks(" << static_cast<void*>(blocks) << "+" << (sizeof(Block)*nBlocks)
    << "), statuses("  << static_cast<void*>(statusBitmap)  << "+" << (sizeof(status_t)*bitmapLen) << ")"
    << std::endl;
#endif
}

BufferPool::~BufferPool()
{
#ifdef _DEBUG
  std::cout << "[BufferPool::~] ";
#endif
  if (statusBitmap) {
#ifdef _DEBUG
    std::cout << "statuses(" << static_cast<void*>(statusBitmap) << ") ";
#endif
    delete[] statusBitmap;  
    statusBitmap = nullptr; 
  }
  if (blocks) { 
#ifdef _DEBUG
    std::cout << "blocks(" << static_cast<void*>(blocks) << ") ";
#endif
    delete[] blocks;
    blocks = nullptr;
  }
  if (buffer) { 
#ifdef _DEBUG
    std::cout << "buffer(" << static_cast<void*>(buffer) << ")";
#endif
    delete[] buffer;        
    buffer = nullptr;
  }
#ifdef _DEBUG
  std::cout << std::endl;
#endif
}

/** Construction/Initialization/Destruction
 *********************************************/


/*********************************************
 * Public methods
 */

pool_buf BufferPool::allocate(size_t size)
{
#ifdef MT_POOL_
  unique_lock<mutex> lock(allocMx);
#endif
  size_t blocksNeeded = (size_t) ceil((float) size / szBlock);
  assert(blocksNeeded > 0);
#ifdef _DEBUG
  std::cout << "[BufferPool::allocate] finding " << blocksNeeded << " blocks" << std::endl;
#endif
  if (blocksNeeded > numBlocksAvailable) {
    std::cerr
      << "Buffer pool out of memory (need " << blocksNeeded
      << ", but only " << numBlocksAvailable << " available)" << std::endl;
    throw std::bad_alloc();
  }

  // Deleted in deallocate() when called from pool_buf::~
  Block** pAllocatedBlocks = new Block*[blocksNeeded];
  size_t blocksFound = findAvailableBlocks(pAllocatedBlocks, blocksNeeded);
  assert(blocksFound == blocksNeeded && "Counted enough available blocks, but can't find them");

  return pool_buf(this, pAllocatedBlocks, blocksFound);
}


void BufferPool::deallocate(const Block* const * pBlocks, size_t count)
{
#ifdef MT_POOL_
  unique_lock<mutex> lock(allocMx);
#endif
  for (size_t i = 0; i < count; ++i) {
    status_t* pStatus = statusBitmap;
    size_t blockIndex = pBlocks[i]->index;
    // Relativize his real index to within one status_t
    while (blockIndex > status_t_size_bits) {
      blockIndex -= status_t_size_bits;
      ++pStatus;
    }
#ifdef _DEBUG
    const int mask = 1 << blockIndex;
    assert(((*pStatus) & mask) == Block::Unavailable && "Deallocated an unallocated block");
#endif
    // Set == available, unset == unavailable
    set_bit(blockIndex, *pStatus);
    ++numBlocksAvailable;
  }

#ifdef _DEBUG
  std::cout
    << "[BufferPool::deallocate] delete[] " << count
    << " blocks("                << static_cast<const void*>(pBlocks) << ")" << std::endl;
#endif
  // This is the array of pointers created in allocate()
  delete[] pBlocks;
}

/** Public methods
 *********************************************/


size_t BufferPool::findAvailableBlocks(BufferPool::Block** pBlocks_out, size_t count)
{
  // Fill back-to-front so we don't have to flip it at the end
  pBlocks_out += count;

  status_t* pStatus = statusBitmap;
  // The index of the first block in the current status word
  size_t baseBlockIndex = 0;
  size_t blocksFound = 0;
  while (blocksFound < count && baseBlockIndex < nBlocks) {
    // Skip to the first status word with an available block
    while ((*pStatus) == Block::Unavailable) {
      ++pStatus;
      baseBlockIndex += status_t_size_bits;
    }
    const size_t relativeIndex = least_set_bit(*pStatus);
    // Mark this block unavailable and take it
    // Iterating will clear the next more significant bits until the word is 0, then skip it.
    clear_bit(relativeIndex, (*pStatus));
    // The block indices are ordered naturally from the blocks pointer
    (*(--pBlocks_out)) = (blocks + baseBlockIndex + relativeIndex);
    ++blocksFound;
    --numBlocksAvailable;

#ifdef _DEBUG
    std::cout
      << "[BufferPool::allocate] block " << (baseBlockIndex + relativeIndex)
      << "(" << static_cast<const void*>(*pBlocks_out) << ")" << std::endl;
#endif
  }
  return blocksFound;
}



/*********************************************
 * BufferPool::Block implementation          */

std::uint8_t& BufferPool::Block::operator [](std::size_t index)
{
  return pBuffer[index];
}

const std::uint8_t& BufferPool::Block::operator [](std::size_t index) const
{
  return pBuffer[index];
}

/** BufferPool::Block implementation
 *********************************************/

}