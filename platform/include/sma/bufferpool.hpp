#pragma once

#include <sma/log>

#include <sma/detail/bits.hpp>
#include <sma/detail/pow2math.hpp>

#include <climits>
#include <cstdlib>
#include <cstdint>
#include <memory>
#include <cstring>    // memcpy, memset
#include <cmath>      // ceil

#define SMA_POOL_MT_

#ifdef SMA_POOL_MT_
#include <mutex>
#endif


// #define RANGE_CHECKED_BUFFERS_


namespace sma
{
template <typename T, std::size_t PageSize>
class BufferPool final
{
  static_assert(PageSize > 0 && ((PageSize & (PageSize - 1)) == 0),
                "Page size must be a power of two.");

  using Myt = BufferPool<T, PageSize>;

  using page_state_t = uint_fast8_t;
  using state_bitmap_t = std::unique_ptr<page_state_t[]>;
  // The number of bits, and thus number of pages' states, in each word of the
  // bitmap
  static const std::size_t states_per_word = sizeof(page_state_t) * CHAR_BIT;
  // Work out the bit counts for divide-by-shift and modulo-by-and given the
  // size of a state word
  using by_states_per_word = Pow2Math<states_per_word>;

  using backing_t = std::unique_ptr<T[]>;

public:
  class Buffer;

  /*******************************************************************************
  * Construction/Initialization
  */

  BufferPool(std::size_t capacity)
    : nr_pages((std::size_t) std::ceil(float(capacity) / PageSize))
    , nr_pages_available(nr_pages)
    , backing_buf(std::move(backing_t(new T[nr_pages * PageSize])))
    , state_bitmap(std::move(make_bitmap(nr_pages)))
  {
  }

  BufferPool(Myt&& move)
    : nr_pages(move.nr_pages)
    , nr_pages_available(move.nr_pages_available)
    , backing_buf(std::move(move.backing_buf))
    , state_bitmap(std::move(move.state_bitmap))
  {
    move.backing_buf = nullptr;
    move.state_bitmap = nullptr;
  }

  Myt& operator=(Myt&& move)
  {
    nr_pages = move.nr_pages;
    nr_pages_available = move.nr_pages_available;

    std::swap(backing_buf, move.backing_buf);
    std::swap(state_bitmap, move.state_bitmap);

    return *this;
  }

  /* Construction/Initialization
   ******************************************************************************/


  /*******************************************************************************
  * Public methods
  */

  Buffer allocate(std::size_t size)
  {
#ifdef SMA_POOL_MT_
    std::unique_lock<std::mutex> lock(state_mutex);
#endif
    const std::size_t nr_pages_needed
        = (std::size_t) std::ceil(float(size) / PageSize);
    assert(nr_pages_needed > 0);

    if (nr_pages_needed > nr_pages_available) {
      LOG(FATAL) << "Buffer pool out of memory (need " << nr_pages_needed
                 << ", but only " << nr_pages_available << " available)";
      throw std::bad_alloc();
    }

    auto allocated_pages = std::unique_ptr<T* []>(new T* [nr_pages_needed]);
    const std::size_t nr_pages_found
        = allocate_pages(allocated_pages.get(), nr_pages_needed);

    assert(nr_pages_found == nr_pages_needed
           && "Counted enough available pages, but can't find them");

    return Buffer(this, std::move(allocated_pages), nr_pages_found);
  }

  /* Public methods
  ******************************************************************************/

private:
  /*! Allocates a bit field of sufficient size to reference each page of the
   * buffer with one
   *  state bit.
   */
  state_bitmap_t static make_bitmap(std::size_t nr_pages)
  {
    const std::size_t bitmap_len
        = (std::size_t) std::ceil((float) nr_pages / states_per_word);

    auto bitmap = std::move(state_bitmap_t(new page_state_t[bitmap_len]));
    // Set the default state of every page to 'available'
    std::memset(bitmap.get(), 0xFF, sizeof(page_state_t) * bitmap_len);
    return std::move(bitmap);
  }

  /*! Find at most \c count available pages and mark them unavailable.
   *  Not thread safe: assumes there are no concurrent allocations or
   * deallocations.
   *  \c count pages must be available at the time of calling to get \c count
   * pages.
   *  \return the number of pages allocated.
   */
  std::size_t allocate_pages(T** pages_out, std::size_t count)
  {
    page_state_t* page_state = state_bitmap.get();

    // The index of the first page in the current state word
    std::size_t base_index = 0;
    std::size_t nr_pages_found = 0;

    while (nr_pages_found < count && base_index < nr_pages) {
      // Skip to the next state word with an available page
      while (!(*page_state)) {
        ++page_state;
        base_index += states_per_word;
      }
      const std::size_t sub_index = least_set_bit(*page_state);
      // Mark this page unavailable and take it
      // Iterating will clear the next more significant bits until the word is
      // 0, then skip it.
      clear_bit(sub_index, *page_state);
      // The page indices are ordered naturally from the pages pointer
      *pages_out++ = (backing_buf.get() + base_index + sub_index);
      ++nr_pages_found;
      --nr_pages_available;
    }
    return nr_pages_found;
  }

  // Called from the pool pointer's destructor
  void deallocate(const T* const* pages, std::size_t count)
  {
#ifdef SMA_POOL_MT_
    std::unique_lock<std::mutex> lock(state_mutex);
#endif
    for (std::size_t i = 0; i < count; ++i) {
      // State indices are naturally ordered from the head of the buffer.
      const std::size_t page_idx = pages[i] - backing_buf.get();
      // page_idx / page_state_t gives which state word the index is in
      page_state_t* page_state = state_bitmap.get()
                                 + (page_idx >> by_states_per_word::shr_to_div);
      // page_idx % page_state_t gives the bit offset into that word
      set_bit(page_idx & by_states_per_word::and_to_mod, *page_state);
      ++nr_pages_available;
    }
  }

  // Guard from accidentally allocating extra memory by passing the class as an
  // argument.
  BufferPool(const Myt& copy) = delete;
  Myt& operator=(const Myt& copy) = delete;

  // Number of pages (given by buffer size / page size)
  std::size_t nr_pages;
  // Number of pages with their state bit set to available
  std::size_t nr_pages_available;
  // This array is allocated in pages by loaning out pointers with constant
  // offsets into it.
  backing_t backing_buf;
  // Each page gets a state bit indicating if it's available for use.
  state_bitmap_t state_bitmap;
#ifdef SMA_POOL_MT_
  // Guard the state bitmap while allocating/deallocating pages.
  std::mutex state_mutex;
#endif



public:
  class Buffer final
  {
    template <typename T_, std::size_t PageSize_>
    friend class BufferPool;

    using pool_t = BufferPool<T, PageSize>;
    using pages_t = std::unique_ptr<T* []>;

    using by_page_size = Pow2Math<PageSize>;

  public:
    Buffer(Buffer&& move)
      : pool(move.pool)
      , pages(std::move(move.pages))
      , nr_pages(move.nr_pages)
      , capacity_(move.capacity_)
      , size_(move.size_)
    {
      move.pool = nullptr;
      move.pages = nullptr;
    }

    ~Buffer()
    {
      if (!pool)
        return;
      pool->deallocate(pages.get(), nr_pages);
    }


    Buffer& operator=(Buffer&& move)
    {
      std::swap(pages, move.pages);
      std::swap(pool, move.pool);
      nr_pages = move.nr_pages;
      capacity_ = move.capacity_;
      size_ = move.size_;
      return *this;
    }

    /*! Copy at most \c count bytes from \c src to this buffer.
     * \return the number of bytes copied.
     */
    std::size_t fill_with(const T* src, std::size_t count)
    {
      if (count > capacity_) {
        count = capacity_;
      }
      T* const* page = pages.get();
      const std::size_t nr_read = count;

      while (count > PageSize) {
        std::memcpy(*page++, src, PageSize);
        count -= PageSize;
        src += PageSize;
      }
      std::memcpy(*page, src, count);

      size_ = nr_read;
      return nr_read;
    }

    /*! Copy at most \c count bytes from this buffer into \c dst.
     * \return the number of bytes copied.
     */
    std::size_t read_into(T* dst, std::size_t count)
    {
      if (count > capacity_) {
        count = capacity_;
      }
      T* const* page = pages.get();
      const std::size_t nr_read = count;

      while (count > PageSize) {
        std::memcpy(dst, *page++, PageSize);
        count -= PageSize;
        dst += PageSize;
      }
      std::memcpy(dst, *page, count);

      return nr_read;
    }

    /*! Release any space allocated by the buffer that is not currently in use.
     * If a memory page is partially used it will not be released.
     * \return the buffer's new capacity.
     */
    std::size_t shrink_to_fit()
    {
      const std::size_t nr_unused_pages = (capacity_ - size_)
                                          >> by_page_size::shr_to_div;
      if (!nr_unused_pages)
        return capacity_;

      nr_pages -= nr_unused_pages;
      T* const* const unused_pages = pages.get() + nr_pages;

      pool->deallocate(unused_pages, nr_unused_pages);

      capacity_ = nr_pages * PageSize;
      return capacity_;
    }

    T& operator[](std::size_t index)
    {
      if (index > size_) {
#ifdef RANGE_CHECKED_BUFFERS_
        if (index >= capacity_) {
          LOG(FATAL)
              << "!!! Fatal error:\n!!! Memory pool buffer index out of range ("
              << index << " >= " << capacity_ << ")";
          std::terminate();
        }
#endif
        size_ = index;
      }
      // Dividing the index by the page size gives us its page index
      // and the remainder is the offset within that page.
      const std::size_t page_index = index >> by_page_size::shr_to_div;
      T* const page = *(pages.get() + page_index);
      return page[index & by_page_size::and_to_mod];
    }

    const T& operator[](std::size_t index) const
    {
#ifdef RANGE_CHECKED_BUFFERS_
      if (index >= capacity_) {
        LOG(FATAL)
            << "!!! Fatal error:\n!!! Memory pool buffer index out of range ("
            << index << " >= " << capacity_ << ")";
        std::terminate();
      }
#endif
      // see T& operator[]
      const std::size_t page_index = index >> by_page_size::shr_to_div;
      T* const page = *(pages.get() + page_index);
      return page[index & by_page_size::and_to_mod];
    }

    std::size_t size() const { return size_; }

    std::size_t capacity() const { return capacity_; }

  private:
    Buffer(pool_t* pool, pages_t pages, std::size_t count)
      : pool(pool)
      , pages(std::move(pages))
      , nr_pages(count)
      , capacity_(count * PageSize)
      , size_(0)
    {
    }

    Buffer(const Buffer& copy) = delete;
    Buffer& operator=(const Buffer& copy) = delete;

    // She owns the pages' memory.
    pool_t* pool;
    // Array of pointers to pages in the pool.
    pages_t pages;
    // The number of page pointers in the array
    std::size_t nr_pages;
    std::size_t capacity_;
    std::size_t size_;
  };    // Buffer

};    // BufferPool

}    // namespace sma
