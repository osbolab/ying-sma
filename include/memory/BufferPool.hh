#pragma once

#include <climits>
#include <cstdlib>
#include <cstdint>
#include <memory>

#ifdef _DEBUG
#include <iostream>
#endif

#define SMA_POOL_MT_

#ifdef SMA_POOL_MT_
#include <mutex>
#endif

#include "Log.hh"
#include "Bits.hh"
#include "Pow2Math.hh"


namespace sma
{

template<typename T, std::size_t BlockSize>
class PooledBuffer;

namespace
{

typedef std::uint_fast8_t block_state_t;

template<typename T, std::size_t BlockSize>
class BufferPool
{
  static_assert(BlockSize > 0 && ((BlockSize & (BlockSize - 1)) == 0),
                "Block size must be a power of two.");

  template<typename T, std::size_t BlockSize>
  friend class PooledBuffer;

public:
  BufferPool(std::size_t capacity);
  BufferPool(BufferPool<T, BlockSize>&& move);
#ifdef _DEBUG
  ~BufferPool();
#endif

  BufferPool<T, BlockSize>&  operator=(BufferPool<T, BlockSize> && move);

  PooledBuffer<T, BlockSize> allocate(std::size_t size);

private:
  // The number of bits, and thus number of blocks' statees, in each word of the bitmap
  static const std::size_t state_t_size_bits;

  std::unique_ptr<T, void (*)(T*)>
  static make_buffer(std::size_t nr_blocks);

  std::unique_ptr<block_state_t, void (*)(block_state_t*)>
  static make_bitmap(std::size_t nr_blocks);

  // Guard from accidentally allocating extra memory by passing the class as an argument.
  BufferPool(const BufferPool<T, BlockSize>& copy)                          = delete;
  BufferPool<T, BlockSize>& operator=(const BufferPool<T, BlockSize>& copy) = delete;

  /* Find at most `count` available blocks and mark them unavailable.
   *  Not thread safe: assumes there are no concurrent allocations or deallocations.
   *  `count` blocks must be available at the time of calling to get `count` blocks.
   */
  std::size_t allocate_blocks(T** blocks_out, std::size_t count);

  // Called from the pool pointer's destructor
  void deallocate(const T* const* blocks, std::size_t count);

  // Number of blocks (given by buffer size / block size)
  std::size_t nr_blocks;
  // Number of blocks with their state bit set to available
  std::size_t nr_blocks_available;
  // This array is allocated in blocks by loaning out pointers with constant offsets into it.
  std::unique_ptr<T, void (*)(T*)> buffer;
  // Each block gets a state bit indicating if it's available for use.
  std::unique_ptr<block_state_t, void (*)(block_state_t*)> state_bitmap;
#ifdef SMA_POOL_MT_
  // Guard the state bitmap while allocating/deallocating blocks.
  std::mutex state_mutex;
#endif
};

}

template<typename T, std::size_t BlockSize>
const std::size_t BufferPool<T, BlockSize>::state_t_size_bits = sizeof(block_state_t)* CHAR_BIT;



/*********************************************
 * Construction/Initialization/Destruction
 */

template<typename T, std::size_t BlockSize>
std::unique_ptr<T, void (*)(T*)>
BufferPool<T, BlockSize>::make_buffer(std::size_t nr_blocks)
{
  auto buffer_deleter = [&](T * ptr) { delete[] ptr; };
  return std::unique_ptr<T, decltype(buffer_deleter)>(
           new T[nr_blocks * BlockSize], buffer_deleter
         );
}


template<typename T, std::size_t BlockSize>
std::unique_ptr<block_state_t, void (*)(block_state_t*)>
BufferPool<T, BlockSize>::make_bitmap(std::size_t nr_blocks)
{
  const std::size_t bitmap_len = (std::size_t) std::ceil((float) nr_blocks / state_t_size_bits);

  auto bitmap_deleter = [&](block_state_t* ptr) { delete[] ptr; };
  auto bitmap = std::move(std::unique_ptr<block_state_t, decltype(bitmap_deleter)>(
                            new block_state_t[bitmap_len], bitmap_deleter
                          ));
  // Set the default state of every block to 'available'
  std::memset(bitmap.get(), 0xFF, sizeof(block_state_t) * bitmap_len);
  return std::move(bitmap);
}


template<typename T, std::size_t BlockSize>
BufferPool<T, BlockSize>::BufferPool(std::size_t capacity)
  : nr_blocks((std::size_t) std::ceil((float)capacity / BlockSize)),
    nr_blocks_available(nr_blocks),
    buffer(std::move(make_buffer(nr_blocks))),
    state_bitmap(std::move(make_bitmap(nr_blocks)))
{
  LOG_D("[BufferPool::()] buffer("
        << static_cast<void*>(buffer.get())       << "+" << (nr_blocks * BlockSize) << ") states("
        << static_cast<void*>(state_bitmap.get()) << ")");
}



template<typename T, std::size_t BlockSize>
BufferPool<T, BlockSize>::BufferPool(BufferPool<T, BlockSize>&& move)
  : buffer(std::move(move.buffer)), state_bitmap(std::move(move.state_bitmap)),
    nr_blocks(move.nr_blocks), nr_blocks_available(move.nr_blocks_available)
{
  LOG_D("[BufferPool::(&&)] " << static_cast<void*>(&move));
  move.buffer       = nullptr;
  move.state_bitmap = nullptr;
}


template<typename T, std::size_t BlockSize>
BufferPool<T, BlockSize>&
BufferPool<T, BlockSize>::operator=(BufferPool<T, BlockSize> && move)
{
  if (&move == this) { return *this; }

  LOG_D("[BufferPool::=(&&)] " << static_cast<void*>(&move));
  nr_blocks           = move.nr_blocks;
  nr_blocks_available = move.nr_blocks_available;

  std::swap(buffer, move.buffer);
  std::swap(state_bitmap, move.state_bitmap);
  return *this;
}


#ifdef _DEBUG
template<typename T, std::size_t BlockSize>
BufferPool<T, BlockSize>::~BufferPool()
{
  LOG_D("[BufferPool::~]");
}
#endif


/** Construction/Initialization/Destruction
 *********************************************/


/*********************************************
 * Public methods
 */

template<typename T, std::size_t BlockSize>
PooledBuffer<T, BlockSize>
BufferPool<T, BlockSize>::allocate(std::size_t size)
{
#ifdef SMA_POOL_MT_
  std::unique_lock<std::mutex> lock(state_mutex);
#endif
  const std::size_t nr_blocks_needed = (std::size_t) std::ceil((float) size / BlockSize);
  assert(nr_blocks_needed > 0);

  if (nr_blocks_needed > nr_blocks_available) {
    std::cerr
        << "Buffer pool out of memory (need " << nr_blocks_needed
        << ", but only " << nr_blocks_available << " available)" << std::endl;
    throw std::bad_alloc();
  }

  LOG_D("[BufferPool::allocate] finding "
        << nr_blocks_needed << " blocks ("
        << nr_blocks_available << " available)");

  // Deleted in deallocate() when called from PooledBuffer::~
  T**         allocated_blocks = new T*[nr_blocks_needed];
  std::size_t nr_blocks_found  = allocate_blocks(allocated_blocks, nr_blocks_needed);

  assert(nr_blocks_found == nr_blocks_needed &&
         "Counted enough available blocks, but can't find them");

  return PooledBuffer<T, BlockSize>(this, allocated_blocks, nr_blocks_found);
}



template<typename T, std::size_t BlockSize>
void
BufferPool<T, BlockSize>::deallocate(const T* const* blocks, std::size_t count)
{
#ifdef SMA_POOL_MT_
  std::unique_lock<std::mutex> lock(state_mutex);
#endif
  for (size_t i = 0; i < count; ++i) {
    // State indices are naturally ordered from the head of the buffer.
    std::size_t block_idx = blocks[i] - buffer.get();
    // block_idx / block_state_t gives which state word the index is in
    block_state_t* block_state = state_bitmap.get() + (block_idx
                                 >> Pow2Math<sizeof(block_state_t)>::div);
    // block_idx % block_state_t gives the bit offset into that word
    set_bit(block_idx & Pow2Math<sizeof(block_state_t)>::mod, *block_state);
    ++nr_blocks_available;

    LOG_D("[BufferPool::deallocate] block "
          << block_idx
          << " (" << static_cast<void*>(buffer.get() + block_idx) << ") (total "
          << nr_blocks_available << ")");
  }
  LOG_D("[BufferPool::deallocate] delete[] block*[" << count << "] ("
        << static_cast<const void*>(blocks) << ")");

  // This is the array of pointers created in allocate()
  delete[] blocks;
}

/** Public methods
 *********************************************/



template<typename T, std::size_t BlockSize>
std::size_t
BufferPool<T, BlockSize>::allocate_blocks(T** blocks_out, std::size_t count)
{
  block_state_t* block_state = state_bitmap.get();

  // The index of the first block in the current state word
  std::size_t base_index      = 0;
  std::size_t nr_blocks_found = 0;

  while (nr_blocks_found < count && base_index < nr_blocks) {
    // Skip to the next state word with an available block
    while (!(*block_state)) {
      ++block_state;
      base_index += state_t_size_bits;
    }
    const std::size_t sub_index = least_set_bit(*block_state);
    // Mark this block unavailable and take it
    // Iterating will clear the next more significant bits until the word is 0, then skip it.
    clear_bit(sub_index, *block_state);
    // The block indices are ordered naturally from the blocks pointer
    *blocks_out++ = (buffer.get() + base_index + sub_index);
    ++nr_blocks_found;
    --nr_blocks_available;

    LOG_D("[BufferPool::allocate_blocks] block " << (base_index + sub_index)
          << " (" << static_cast<void*>(*(blocks_out - 1)) << ")");
  }
  return nr_blocks_found;
}

}