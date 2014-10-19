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

#include "log.hh"
#include "bits.hh"
#include "pow2_math.hh"


namespace sma
{

template<typename T, std::size_t BlockSize>
class buffer_pool
{
  static_assert(BlockSize > 0 && ((BlockSize & (BlockSize-1)) == 0),
                "Block size must be a power of two.");

  template<typename T, std::size_t BlockSize>
  friend class pool_buf;

public:
  buffer_pool(std::size_t capacity);
  buffer_pool(buffer_pool<T, BlockSize>&& move);
  ~buffer_pool();

  buffer_pool<T, BlockSize>& operator=(buffer_pool<T, BlockSize>&& move);

  pool_buf<T, BlockSize> allocate(std::size_t size);

private:
  // The type composing the state bitmap
  typedef       std::uint_fast8_t state_t;
  // The number of bits, and thus number of blocks' statees, in each word of the bitmap
  static const  std::size_t       state_t_size_bits;

  // Guard from accidentally allocating extra memory by passing the class as an argument.
  buffer_pool(const buffer_pool<T, BlockSize>& copy) = delete;
  buffer_pool<T, BlockSize>& operator =(const buffer_pool<T, BlockSize>& copy) = delete;

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
  T*          buffer;
  // Each block gets a state bit indicating if it's available for use.
  state_t*    state_bitmap;
#ifdef SMA_POOL_MT_
  // Guard the state bitmap while allocating/deallocating blocks.
  std::mutex  state_mutex;
#endif
};


template<typename T, std::size_t BlockSize>
const std::size_t buffer_pool<T, BlockSize>::state_t_size_bits = sizeof(state_t) * CHAR_BIT;




/*********************************************
* Construction/Initialization/Destruction
*/

template<typename T, std::size_t BlockSize>
buffer_pool<T, BlockSize>::buffer_pool(std::size_t capacity)
  : // Increase the capacity to fit only whole blocks
  nr_blocks((std::size_t) std::ceil((float) capacity / BlockSize)),
  nr_blocks_available(nr_blocks),
  buffer(new T[nr_blocks * BlockSize]),
  state_bitmap(nullptr)
{
  const std::size_t bitmap_len = (std::size_t) std::ceil((float) nr_blocks / state_t_size_bits);
  state_bitmap = new state_t[bitmap_len];
  // Set the default state of every block to 'available'
  std::memset(state_bitmap, 0xFF, sizeof(state_t) * bitmap_len);

  LOG_D("[buffer_pool::()] buffer("
        << static_cast<void*>(buffer)       << "+" << (nr_blocks * BlockSize)       << ") states("
        << static_cast<void*>(state_bitmap) << "+" << (sizeof(state_t)*bitmap_len)  << ")");
}



template<typename T, std::size_t BlockSize>
buffer_pool<T, BlockSize>::buffer_pool(buffer_pool<T, BlockSize>&& move)
  : buffer(move.buffer), state_bitmap(move.state_bitmap),
  nr_blocks(move.nr_blocks), nr_blocks_available(move.nr_blocks_available)
{
  LOG_D("[buffer_pool::(&&)] " << static_cast<void*>(&move));
  move.buffer = nullptr;
  move.state_bitmap = nullptr;
}



template<typename T, std::size_t BlockSize>
buffer_pool<T, BlockSize>::~buffer_pool()
{
  LOG_D("[buffer_pool::~]");
  delete[] buffer;
  delete[] state_bitmap;
}


/** Construction/Initialization/Destruction
*********************************************/


/*********************************************
* Public methods
*/

template<typename T, std::size_t BlockSize>
pool_buf<T, BlockSize>
buffer_pool<T, BlockSize>::allocate(std::size_t size)
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

  LOG_D("[buffer_pool::allocate] finding " 
        << nr_blocks_needed << " blocks (" 
        << nr_blocks_available << " available)");

  // Deleted in deallocate() when called from pool_buf::~
  T** allocated_blocks = new T*[nr_blocks_needed];
  std::size_t nr_blocks_found = allocate_blocks(allocated_blocks, nr_blocks_needed);
  assert(nr_blocks_found == nr_blocks_needed && 
         "Counted enough available blocks, but can't find them");

  return pool_buf<T, BlockSize>(this, allocated_blocks, nr_blocks_found);
}



template<typename T, std::size_t BlockSize>
void 
buffer_pool<T, BlockSize>::deallocate(const T* const * blocks, std::size_t count)
{
#ifdef SMA_POOL_MT_
  std::unique_lock<std::mutex> lock(state_mutex);
#endif
  for (size_t i = 0; i < count; ++i) {
    // State indices are naturally ordered from the head of the buffer.
    std::size_t block_idx = blocks[i] - buffer;
    // block_idx / state_t gives which state word the index is in
    state_t* block_state = state_bitmap + (block_idx >> pow2_math<sizeof(state_t)>::div);
    // block_idx % state_t gives the bit offset into that word
    set_bit(block_idx & pow2_math<sizeof(state_t)>::mod, *block_state);
    ++nr_blocks_available;
    LOG_D("[buffer_pool::deallocate] block " << block_idx
          << " (" << static_cast<void*>(buffer + block_idx) << ") (total " 
          << nr_blocks_available << ")");
  }
  LOG_D("[buffer_pool::deallocate] delete[] block*[" << count << "] ("
        << static_cast<const void*>(blocks) << ")");

  // This is the array of pointers created in allocate()
  delete[] blocks;
}



template<typename T, std::size_t BlockSize>
buffer_pool<T, BlockSize>&
buffer_pool<T, BlockSize>::operator=(buffer_pool<T, BlockSize>&& move)
{
  LOG_D("[buffer_pool::=(&&)] " << static_cast<void*>(&move));
  nr_blocks = move.nr_blocks;
  nr_blocks_available = move.nr_blocks_available;

  std::swap(buffer, move.buffer);
  std::swap(state_bitmap, move.state_bitmap);
  return *this;
}

/** Public methods
*********************************************/



template<typename T, std::size_t BlockSize>
std::size_t 
buffer_pool<T, BlockSize>::allocate_blocks(T** blocks_out, std::size_t count)
{
  state_t* block_state = state_bitmap;
  // The index of the first block in the current state word
  std::size_t base_index = 0;
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
    *blocks_out++ = (buffer + base_index + sub_index);
    ++nr_blocks_found;
    --nr_blocks_available;
    LOG_D("[buffer_pool::allocate_blocks] block " << (base_index + sub_index) 
          << " (" << static_cast<void*>(*(blocks_out-1)) << ")");
  }
  return nr_blocks_found;
}

}