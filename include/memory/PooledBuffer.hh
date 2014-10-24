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

public:
  PooledBuffer(PooledBuffer&& move);
  ~PooledBuffer();

  PooledBuffer& operator =(PooledBuffer&& move);

  /*! Copy at most \c count bytes from \c src to this buffer.
   * \return the number of bytes copied.
   */
  std::size_t fill_with(const T* src, std::size_t count);

  /*! Copy at most \c count bytes from this buffer into \c dst.
   * \return the number of bytes copied.
   */
  std::size_t read_into(T* dst, std::size_t count);

  /*! Release any space allocated by the buffer that is not currently in use.
   * If a memory page is partially used it will not be released.
   * \return the buffer's new capacity.
   */
  std::size_t shrink_to_fit();

  T& operator[](std::size_t index);
  const T& operator[](std::size_t index) const;

  std::size_t get_size() const { return size; }
  std::size_t get_capacity() const { return capacity; }

private:
  PooledBuffer(BufferPool<T, PageSize>* pool,
               std::unique_ptr<T*[]> pages,
               std::size_t count);

  PooledBuffer(const PooledBuffer& copy) = delete;
  PooledBuffer& operator =(const PooledBuffer& copy) = delete;

  // She owns the pages' memory.
  BufferPool<T, PageSize>* pool;
  // Array of pointers to pages in the pool.
  std::unique_ptr<T*[]> pages;
  // The number of page pointers in the array
  std::size_t nr_pages;
  std::size_t capacity;
  std::size_t size;
};


template<typename T, std::size_t PageSize>
PooledBuffer<T, PageSize>::PooledBuffer(BufferPool<T, PageSize>* pool,
                                        std::unique_ptr<T*[]> pages,
                                        std::size_t count)
  : pool(pool), pages(std::move(pages)), nr_pages(count), capacity(count* PageSize), size(0)
{
  LOG_D("[PooledBuffer::()] page*[" << nr_pages << "] ("
        << static_cast<const void*>(this->pages.get()) << ")");
}


template<typename T, std::size_t PageSize>
PooledBuffer<T, PageSize>::PooledBuffer(PooledBuffer<T, PageSize>&& move)
  : pool(move.pool), pages(std::move(move.pages)), nr_pages(move.nr_pages),
    capacity(move.capacity), size(move.size)
{
  LOG_D("[PooledBuffer::(&&)] " << static_cast<void*>(&move));
  move.pool = nullptr;
  move.pages = nullptr;
}


template<typename T, std::size_t PageSize>
PooledBuffer<T, PageSize>::~PooledBuffer()
{
  if (!pool) return;
  LOG_D("[PooledBuffer::~]");
  pool->deallocate(pages.get(), nr_pages);
}


template<typename T, std::size_t PageSize>
PooledBuffer<T, PageSize>&
PooledBuffer<T, PageSize>::operator=(PooledBuffer<T, PageSize>&& move)
{
  LOG_D("[PooledBuffer::=(&&)] " << static_cast<void*>(&move));
  std::swap(pages, move.pages);
  std::swap(pool, move.pool);
  nr_pages = move.nr_pages;
  capacity  = move.capacity;
  size      = move.size;
  return (*this);
}


template<typename T, std::size_t PageSize>
std::size_t
PooledBuffer<T, PageSize>::shrink_to_fit()
{
  std::size_t nr_unused_pages = (capacity - size) >> Pow2Math<PageSize>::div;
  LOG_D("[PooledBuffer::shrink_to_fit] " << nr_unused_pages << " unused pages");
  if (!nr_unused_pages) return capacity;
  LOG_D("[PooledBuffer::shrink_to_fit] shrink from " << capacity << " bytes");
  nr_pages -= nr_unused_pages;
  capacity = nr_pages * PageSize;
  T* const* unused_pages = pages.get() + nr_pages;
  pool->deallocate(unused_pages, nr_unused_pages);
  LOG_D("[PooledBuffer::shrink_to_fit] shrunk to " << capacity << " bytes");
  return capacity;
}


template<typename T, std::size_t PageSize>
std::size_t
PooledBuffer<T, PageSize>::fill_with(const T* src, std::size_t count)
{
  if (count > capacity) { count = capacity; }
  T* const* page = pages.get();
  const size_t nr_read = count;

  while (count > PageSize) {
    std::memcpy(*page++, src, PageSize);
    count -= PageSize;
    src += PageSize;
  }
  std::memcpy(*page, src, count);

  size = nr_read;
  return nr_read;
}


template<typename T, std::size_t PageSize>
std::size_t
PooledBuffer<T, PageSize>::read_into(T* dst, std::size_t count)
{
  if (count > capacity) { count = capacity; }
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


template<typename T, std::size_t PageSize>
T&
PooledBuffer<T, PageSize>::operator[](std::size_t index)
{
  if (index > size) {
#ifdef RANGE_CHECKED_BUFFERS_
    if (index >= capacity) {
      std::cerr
          << "!!! Fatal error:\n!!!     Memory pool buffer index out of range ("
          << index << " >= " << capacity << ")" << std::endl;
      std::terminate();
    }
#endif
    size = index;
  }
  // Dividing the index by the page size gives us its page index
  // and the remainder is the offset within that page.
  return (*(pages.get() + (index >> Pow2Math<PageSize>::div)))[index & Pow2Math<PageSize>::mod];
}


template<typename T, std::size_t PageSize>
const T&
PooledBuffer<T, PageSize>::operator[](std::size_t index) const
{
#ifdef RANGE_CHECKED_BUFFERS_
  if (index >= capacity) {
    std::cerr
        << "!!! Fatal error:\n!!! Memory pool buffer index out of range ("
        << index << " >= " << capacity << ")" << std::endl;
    std::terminate();
  }
#endif
  // see T& operator[]
  return (*(pages.get() + (index >> Pow2Math<PageSize>::div)))[index & Pow2Math<PageSize>::mod];
}

}