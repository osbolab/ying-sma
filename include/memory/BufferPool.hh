#pragma once

#include <climits>
#include <cstdlib>
#include <cstdint>
#include <memory>

#define MT_POOL_

#ifdef MT_POOL_
#include <mutex>
#endif


namespace sma
{

template<class T, std::size_t szBlock>
class BufferPool
{
  friend class pool_buf<T, szBlock>;

public:
  static std::unique_ptr<BufferPool> create(std::size_t capacity);

  BufferPool(BufferPool&& move);
  BufferPool& operator =(BufferPool&& move);

  ~BufferPool();

  pool_buf allocate(std::size_t size);

private:
  //! Initialize the pool's parameters, but don't allocate any memory.
  BufferPool(std::size_t capacity, std::size_t blockSize);
  //! Allocate the backing arrays.
  void initialize();

  /*! Find at most `count` available blocks and mark them unavailable.
   *  Not thread safe: assumes there are no concurrent allocations or deallocations.
   *  `count` blocks must be available at the time of calling to get `count` blocks.
   */
  std::size_t findAvailableBlocks(T** pBlocks_out, size_t count);

  //! Called from the pool pointer's destructor
  void deallocate(const T* const* pBlocks, std::size_t count);

  // Number of blocks according to (buffer size / block size)
  std::size_t  nBlocks;
  // The total number of blocks with their status bit set to available
  std::size_t  numBlocksAvailable;

  // The type composing the status bitmap
  typedef       std::uint_fast8_t status_t;
  // The number of bits, and thus number of blocks' statuses, in each word of the bitmap
  static const  std::size_t       status_t_size_bits;

  // These arrays must be deleted at deconstruction
  //--------------------------------
  // Each block gets a status bit indicating if it's available for use.
  status_t*   statusBitmap;
  // This array is allocated in chunks by loaning out pointers with constant offsets into it.
  T*          buffer;
  // Each block points to a section of the backing array and is loaned out by allocate().
  Block*      blocks;
  //--------------------------------

#ifdef MT_POOL_
  // Guard the status bitmap while allocating/deallocating blocks
  std::mutex  allocMx;
#endif

  // Guard from accidentally allocating extra memory by passing an argument.
  BufferPool(const BufferPool& copy) = delete;
  BufferPool& operator =(const BufferPool& copy) = delete;
};


template<class T, std::size_t szBlock>
const size_t BufferPool<T, szBlock>::status_t_size_bits = sizeof(status_t) * CHAR_BIT;


/*********************************************
* Static factory methods
*/

template<class T, std::size_t szBlock>
std::unique_ptr<BufferPool<T>> BufferPool<T>::create(size_t capacity)
{
  auto pool = BufferPool<T, szBlock>(capacity);
  pool.initialize();
  return pool;
}

/** Static factory methods
*********************************************/

}