#pragma once

#include "Log.hh"
#include "Bits.hh"
#include "Pow2Math.hh"

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


namespace sma
{

template<typename T, std::size_t PageSize>
class PooledBuffer;

namespace
{

using page_state_t = std::uint_fast8_t;
// The number of bits, and thus number of pages' states, in each word of the bitmap
const std::size_t state_t_size_bits = sizeof(page_state_t)* CHAR_BIT;


template<typename T, std::size_t PageSize>
class BufferPool final
{
  static_assert(PageSize > 0 && ((PageSize & (PageSize - 1)) == 0),
                "Page size must be a power of two.");

  template<typename T, std::size_t PageSize>
  friend class PooledBuffer;

public:
  BufferPool(std::size_t capacity);
  BufferPool(BufferPool<T, PageSize>&& move);
#ifdef _DEBUG
  ~BufferPool();
#endif

  BufferPool<T, PageSize>&  operator=(BufferPool<T, PageSize> && move);

  PooledBuffer<T, PageSize> allocate(std::size_t size);

private:
  std::unique_ptr<T, void (*)(T*)>
  static make_buffer(std::size_t nr_pages);

  std::unique_ptr<page_state_t, void (*)(page_state_t*)>
  static make_bitmap(std::size_t nr_pages);

  // Guard from accidentally allocating extra memory by passing the class as an argument.
  BufferPool(const BufferPool<T, PageSize>& copy)                          = delete;
  BufferPool<T, PageSize>& operator=(const BufferPool<T, PageSize>& copy) = delete;

  /* Find at most `count` available pages and mark them unavailable.
   *  Not thread safe: assumes there are no concurrent allocations or deallocations.
   *  `count` pages must be available at the time of calling to get `count` pages.
   */
  std::size_t allocate_pages(T** pages_out, std::size_t count);

  // Called from the pool pointer's destructor
  void deallocate(const T* const* pages, std::size_t count);

  // Number of pages (given by buffer size / page size)
  std::size_t nr_pages;
  // Number of pages with their state bit set to available
  std::size_t nr_pages_available;
  // This array is allocated in pages by loaning out pointers with constant offsets into it.
  std::unique_ptr<T, void (*)(T*)> buffer;
  // Each page gets a state bit indicating if it's available for use.
  std::unique_ptr<page_state_t, void (*)(page_state_t*)> state_bitmap;
#ifdef SMA_POOL_MT_
  // Guard the state bitmap while allocating/deallocating pages.
  std::mutex state_mutex;
#endif
};

}


/*******************************************************************************
 * Construction/Initialization/Destruction
 */

template<typename T, std::size_t PageSize>
std::unique_ptr<T, void (*)(T*)>
BufferPool<T, PageSize>::make_buffer(std::size_t nr_pages)
{
  auto buffer_deleter = [&](T * ptr) { delete[] ptr; };
  return std::unique_ptr<T, decltype(buffer_deleter)>(
           new T[nr_pages * PageSize], buffer_deleter
         );
}


template<typename T, std::size_t PageSize>
std::unique_ptr<page_state_t, void (*)(page_state_t*)>
BufferPool<T, PageSize>::make_bitmap(std::size_t nr_pages)
{
  const std::size_t bitmap_len = (std::size_t) std::ceil((float) nr_pages / state_t_size_bits);

  auto bitmap_deleter = [&](page_state_t* ptr) { delete[] ptr; };
  auto bitmap = std::move(std::unique_ptr<page_state_t, decltype(bitmap_deleter)>(
                            new page_state_t[bitmap_len], bitmap_deleter
                          ));
  // Set the default state of every page to 'available'
  std::memset(bitmap.get(), 0xFF, sizeof(page_state_t) * bitmap_len);
  return std::move(bitmap);
}


template<typename T, std::size_t PageSize>
BufferPool<T, PageSize>::BufferPool(std::size_t capacity)
  : nr_pages((std::size_t) std::ceil((float)capacity / PageSize)),
    nr_pages_available(nr_pages),
    buffer(std::move(make_buffer(nr_pages))),
    state_bitmap(std::move(make_bitmap(nr_pages)))
{
  LOG_D("[BufferPool::()] buffer("
        << static_cast<void*>(buffer.get())       << "+" << (nr_pages * PageSize) << ") states("
        << static_cast<void*>(state_bitmap.get()) << ")");
}



template<typename T, std::size_t PageSize>
BufferPool<T, PageSize>::BufferPool(BufferPool<T, PageSize>&& move)
  : buffer(std::move(move.buffer)), state_bitmap(std::move(move.state_bitmap)),
    nr_pages(move.nr_pages), nr_pages_available(move.nr_pages_available)
{
  LOG_D("[BufferPool::(&&)] " << static_cast<void*>(&move));
  move.buffer       = nullptr;
  move.state_bitmap = nullptr;
}


template<typename T, std::size_t PageSize>
BufferPool<T, PageSize>&
BufferPool<T, PageSize>::operator=(BufferPool<T, PageSize> && move)
{
  if (&move == this) { return *this; }

  LOG_D("[BufferPool::=(&&)] " << static_cast<void*>(&move));
  nr_pages           = move.nr_pages;
  nr_pages_available = move.nr_pages_available;

  std::swap(buffer, move.buffer);
  std::swap(state_bitmap, move.state_bitmap);
  return *this;
}


#ifdef _DEBUG
template<typename T, std::size_t PageSize>
BufferPool<T, PageSize>::~BufferPool()
{
  LOG_D("[BufferPool::~]");
}
#endif


/* Construction/Initialization/Destruction
 ******************************************************************************/


/*******************************************************************************
 * Public methods
 */

template<typename T, std::size_t PageSize>
PooledBuffer<T, PageSize>
BufferPool<T, PageSize>::allocate(std::size_t size)
{
#ifdef SMA_POOL_MT_
  std::unique_lock<std::mutex> lock(state_mutex);
#endif
  const std::size_t nr_pages_needed = (std::size_t) std::ceil((float) size / PageSize);
  assert(nr_pages_needed > 0);

  if (nr_pages_needed > nr_pages_available) {
    std::cerr
        << "Buffer pool out of memory (need " << nr_pages_needed
        << ", but only " << nr_pages_available << " available)" << std::endl;
    throw std::bad_alloc();
  }

  LOG_D("[BufferPool::allocate] finding "
        << nr_pages_needed << " pages ("
        << nr_pages_available << " available)");

  auto allocated_pages = std::unique_ptr<T*[]>(new T*[nr_pages_needed]);

  std::size_t nr_pages_found  = allocate_pages(allocated_pages.get(), nr_pages_needed);

  assert(nr_pages_found == nr_pages_needed &&
         "Counted enough available pages, but can't find them");

  return PooledBuffer<T, PageSize>(this, std::move(allocated_pages), nr_pages_found);
}



template<typename T, std::size_t PageSize>
void
BufferPool<T, PageSize>::deallocate(const T* const* pages, std::size_t count)
{
#ifdef SMA_POOL_MT_
  std::unique_lock<std::mutex> lock(state_mutex);
#endif
  for (size_t i = 0; i < count; ++i) {
    // State indices are naturally ordered from the head of the buffer.
    std::size_t page_idx = pages[i] - buffer.get();
    // page_idx / page_state_t gives which state word the index is in
    page_state_t* page_state = state_bitmap.get() + (page_idx
                               >> Pow2Math<state_t_size_bits>::div);
    // page_idx % page_state_t gives the bit offset into that word
    set_bit(page_idx & Pow2Math<state_t_size_bits>::mod, *page_state);
    ++nr_pages_available;

    LOG_D("[BufferPool::deallocate] page "
          << page_idx
          << " (" << static_cast<void*>(buffer.get() + page_idx) << ") (total "
          << nr_pages_available << ")");
  }
}

/* Public methods
 ******************************************************************************/



template<typename T, std::size_t PageSize>
std::size_t
BufferPool<T, PageSize>::allocate_pages(T** pages_out, std::size_t count)
{
  page_state_t* page_state = state_bitmap.get();

  // The index of the first page in the current state word
  std::size_t base_index      = 0;
  std::size_t nr_pages_found = 0;

  while (nr_pages_found < count && base_index < nr_pages) {
    // Skip to the next state word with an available page
    while (!(*page_state)) {
      ++page_state;
      base_index += state_t_size_bits;
    }
    const std::size_t sub_index = least_set_bit(*page_state);
    // Mark this page unavailable and take it
    // Iterating will clear the next more significant bits until the word is 0, then skip it.
    clear_bit(sub_index, *page_state);
    // The page indices are ordered naturally from the pages pointer
    *pages_out++ = (buffer.get() + base_index + sub_index);
    ++nr_pages_found;
    --nr_pages_available;

    LOG_D("[BufferPool::allocate_pages] page " << (base_index + sub_index)
          << " (" << static_cast<void*>(*(pages_out - 1)) << ")");
  }
  return nr_pages_found;
}

}