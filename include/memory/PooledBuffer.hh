#pragma once

#include "util/Log.hh"
#include "util/Pow2Math.hh"

#include <iostream>
#include <cstdlib>
#include <string>

// #define RANGE_CHECKED_BUFFERS_

namespace sma
{

template<typename T, std::size_t PageSize>
class PooledBuffer final
{
  static_assert(PageSize > 0 && ((PageSize & (PageSize-1)) == 0),
                "Page size must be a power of two.");

  template<typename T, std::size_t PageSize>
  friend class BufferPool;

  using MyT = PooledBuffer<T, PageSize>;

  using pool_t = BufferPool<T, PageSize>;
  using pages_t = std::unique_ptr<T*[]>;

  using by_page_size = Pow2Math<PageSize>;

public:
  PooledBuffer(MyT&& move)
    : pool(move.pool),
      pages(std::move(move.pages)),
      nr_pages(move.nr_pages),
      capacity_(move.capacity_),
      size_(move.size_)
  {
    move.pool = nullptr;
    move.pages = nullptr;
  }

  ~PooledBuffer()
  {
    if (!pool) return;
    pool->deallocate(pages.get(), nr_pages);
  }


  MyT& operator =(MyT&& move)
  {
    std::swap(pages, move.pages);
    std::swap(pool, move.pool);
    nr_pages = move.nr_pages;
    capacity_  = move.capacity_;
    size_      = move.size_;
    return *this;
  }

  /*! Copy at most \c count bytes from \c src to this buffer.
   * \return the number of bytes copied.
   */
  std::size_t fill_with(const T* src, std::size_t count)
  {
    if (count > capacity_) { count = capacity_; }
    T* const* page = pages.get();
    const size_t nr_read = count;

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
    if (count > capacity_) { count = capacity_; }
    T* const* page = pages.get();
    const size_t nr_read = count;

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
    const std::size_t nr_unused_pages = (capacity_ - size_) >> by_page_size::shr_to_div;
    if (!nr_unused_pages) return capacity_;

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
        std::cerr
            << "!!! Fatal error:\n!!! Memory pool buffer index out of range ("
            << index << " >= " << capacity_ << ")" << std::endl;
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
      std::cerr
          << "!!! Fatal error:\n!!! Memory pool buffer index out of range ("
          << index << " >= " << capacity_ << ")" << std::endl;
      std::terminate();
    }
#endif
    // see T& operator[]
    const std::size_t page_index = index >> by_page_size::shr_to_div;
    T* const page = *(pages.get() + page_index);
    return page[index & by_page_size::and_to_mod];
  }

  std::size_t size() const
  {
    return size_;
  }

  std::size_t capacity() const
  {
    return capacity_;
  }

private:
  PooledBuffer(pool_t* pool, pages_t pages, std::size_t count)
    : pool(pool),
      pages(std::move(pages)),
      nr_pages(count),
      capacity_(count* PageSize),
      size_(0)
  {
  }

  PooledBuffer(const MyT& copy) = delete;
  MyT& operator =(const MyT& copy) = delete;

  // She owns the pages' memory.
  pool_t* pool;
  // Array of pointers to pages in the pool.
  pages_t pages;
  // The number of page pointers in the array
  std::size_t nr_pages;
  std::size_t capacity_;
  std::size_t size_;
};

}