#pragma once

#include <cstdlib>
#include <cstdint>
#include <memory>

#define MT_BUFFER_POOL_

#ifdef MT_BUFFER_POOL_
#include <mutex>
#endif


namespace sma
{

class pool_ptr;

class BufferPool
{
  friend class pool_ptr;

public:
  static std::unique_ptr<BufferPool> create(std::size_t capacity);
  ~BufferPool();

  pool_ptr allocate(std::size_t size);

private:
  struct Block
  {
    enum Status { Unavailable = 0, Available = 1 };

    Block(std::size_t index, std::uint8_t* pBuffer, std::size_t size)
      : index(index), pBuffer(pBuffer)
#ifdef _DEBUG
      , size(size) 
#endif
    {}
    // The bit in the status bitmap that contains this block's status
    std::size_t   const index;
    // Indexed into the pool's buffer at the head of this block.
    // The pool owns this pointer.
    std::uint8_t* const pBuffer;
#ifdef _DEBUG
    // Used for range checking accessors
    std::size_t   const size;
#endif

    std::uint8_t& operator [](std::size_t index);
    const std::uint8_t& operator [](std::size_t index) const;
  };


  BufferPool(std::size_t capacity, std::size_t blockSize);
  void initialize();

  void deallocate(Block** blocks, std::size_t count);


  BufferPool(const BufferPool& copy) = delete;
  BufferPool& operator =(const BufferPool& copy) = delete;

  // The size of the buffer available to each block
  const std::size_t  szBlock;
  // The total size of the data buffer
  const std::size_t  bufferLen;
  // Number of blocks according to (buffer size / block size)
  const std::size_t  nBlocks;
  // The number of `status_t` that we need to give every block her own status bit
  const std::size_t  bitmapLen;

  typedef       std::uint32_t status_t;
  static const  std::size_t   status_t_size_bits;

  // Each block gets a status bit indicating if it's available for use.
  status_t*          statusBitmap;
  // We'll index into this array according to the status bitmap to pick blocks
  Block*             blocks;
  // Every block just holds a pointer offset into this array
  std::uint8_t*      buffer;
  // The total number of blocks with their status bit set to available
  std::size_t        blocksAvailable;

#ifdef MT_BUFFER_POOL_
  // Guard the status bitmap while allocating/deallocating blocks
  std::mutex          allocMx;
#endif

};

}