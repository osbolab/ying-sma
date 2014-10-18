#include <utility>
#include <cstring>
#include <cmath>
#include <memory>
#include <iostream>
#include <cassert>

#include "memory/bits.hh"

#define MT_POOL_

#include "memory/pool_ptr.hh"
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


const size_t BufferPool::status_t_size_bits = 8 * sizeof(status_t);


unique_ptr<BufferPool> BufferPool::create(size_t capacity)
{
  auto ptr = unique_ptr<BufferPool>(new BufferPool(capacity, 16));
  ptr->initialize();
  return ptr;
}

BufferPool::BufferPool(size_t capacity, size_t blockSize)
  : statusBitmap(nullptr), blocks(nullptr), buffer(nullptr),
  // Keep the block size as a power of two for alignment
  szBlock(1 << (ms_bit(blockSize) + 1)),
  // Take the next largest size that fits only whole blocks
  bufferLen((size_t) std::ceil((float) capacity / szBlock)*szBlock),
  nBlocks(bufferLen / szBlock),
  // One status bit per block
  bitmapLen((size_t) std::ceil((float)nBlocks / status_t_size_bits)),
  blocksAvailable(0)
{
#ifdef _DEBUG
  std::cout
    << "[BufferPool::()] buffer: "  << bufferLen
    << " B, block: "                << szBlock
    << " B, blocks: "               << nBlocks
    << ", index: "                  << (bitmapLen * status_t_size_bits) << " B";
#ifdef MT_POOL_
  std::cout                         << ", multithreaded";
#endif
  std::cout << std::endl;
#endif
}

pool_ptr BufferPool::allocate(size_t size)
{
#ifdef MT_POOL_
  unique_lock<mutex> lock(allocMx);
#endif

  size_t blocksNeeded = (size_t) ceil((float) size / szBlock);
  assert(blocksNeeded > 0);

#ifdef _DEBUG
  std::cout << "[BufferPool::allocate] finding " << blocksNeeded << " blocks" << std::endl;
#endif

  if (blocksNeeded > blocksAvailable) {
    std::cerr
      << "Buffer pool out of memory (" << blocksAvailable << " / " << nBlocks
      << " blocks available)" << std::endl;
    return pool_ptr::null();
  }

  // Deleted in deallocate() after being passed by pool_ptr::~
  Block** pAllocatedBlocks = new Block*[blocksNeeded];
  // Work backwards so we don't have to flip it at the end
  pAllocatedBlocks += blocksNeeded;

  status_t* pStatus = statusBitmap;
  // Remember the indices we skipped without recounting every time
  size_t blockIndex = 0;
  size_t blocksFound = 0;
  while (blocksFound < blocksNeeded) {
    // Skip whole words of totally unavailable indices
    while (*(pStatus) == Block::Unavailable) { 
      ++pStatus; 
      blockIndex += status_t_size_bits;
      // The blocksAvailable counter indicated that we would find enough blocks;
      // I probably screwed something up with concurrency if this happens.
      if (blockIndex >= nBlocks) {
        assert(false && "Reached the end of the status bitmap without finding enough blocks.");
      }
    }
    // The index of the first high bit is the index of our block
    const size_t firstSetBit = ls_bit(*pStatus);
    // Make this block unavailable and take it
    clear_bit(firstSetBit, *pStatus);
    assert(ls_bit(*pStatus) != firstSetBit && "Failed to clear the block's status bit");
    blockIndex += firstSetBit;
    // The indices are in order from the base blocks pointer
    *(--pAllocatedBlocks) = (blocks + blockIndex);
    --blocksAvailable;
    ++blocksFound;

#ifdef _DEBUG
    std::cout 
      << "[BufferPool::allocate] allocated block #" << blockIndex 
      << " (need " << (blocksNeeded - blocksFound)
      << " more of the available " << blocksAvailable << ")" << std::endl;
#endif
    // The next round will pick the next more significant bit until the word is zero,
    // and thus work through the list.
  }

  return pool_ptr(this, pAllocatedBlocks, blocksFound);
}

void BufferPool::deallocate(Block** pBlocks, size_t count)
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
    assert((*pStatus & mask) == Block::Unavailable && "Deallocated an unallocated block");
#endif
    // Set == available, unset == unavailable
    set_bit(blockIndex, *pStatus);
    ++blocksAvailable;
  }

#ifdef _DEBUG
  std::cout
    << "[BufferPool::delete[]] " << count
    << " blocks("                << static_cast<void*>(pBlocks) << ")" << std::endl;
#endif
  // This is the array of pointers created in allocate()
  delete[] pBlocks;
}

void BufferPool::initialize()
{
  if (statusBitmap || blocks || buffer) return;

  buffer = new uint8_t[bufferLen];

  // Set the default status of every block to 'available'
  statusBitmap = new status_t[bitmapLen];
  std::memset(statusBitmap, 0xFF, sizeof(status_t) * bitmapLen);

  // Construct and initialize blocks in place
  blocks = (Block*) new uint8_t[sizeof(Block) * nBlocks];
  Block* p = blocks;
  for (size_t i = 0; i < nBlocks; ++i) {
    new (p++) Block(i, buffer + (i * sizeof(Block)), szBlock);
  }
  blocksAvailable = nBlocks;

#ifdef _DEBUG
  std::cout
    << "[BufferPool::init] buffer("
    <<                 static_cast<void*>(buffer) << "+" << bufferLen
    << "), blocks(" << static_cast<void*>(blocks) << "+" << (sizeof(Block)*nBlocks)
    << "), index("  << static_cast<void*>(statusBitmap)  << "+" << (sizeof(status_t)*bitmapLen) << ")"
    << std::endl;
#endif
}

BufferPool::~BufferPool()
{
#ifdef _DEBUG
  std::cout << "[BufferPool::~]" << std::endl;
#endif
  if (statusBitmap)   delete[] statusBitmap;  statusBitmap  = nullptr;
  if (blocks)         delete[] blocks;        blocks        = nullptr;
  if (buffer)         delete[] buffer;        buffer        = nullptr;
}

std::uint8_t& BufferPool::Block::operator [](std::size_t index)
{
#ifdef _DEBUG
  assert(index < size && "Block buffer index out of range");
#endif
  return pBuffer[index];
}

const std::uint8_t& BufferPool::Block::operator [](std::size_t index) const
{
#ifdef _DEBUG
  assert(index < size && "Block buffer index out of range");
#endif
  return pBuffer[index];
}

}