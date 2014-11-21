#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>


namespace sma
{
namespace detail
{
  class BufferView
  {
    friend class Buffer;

  public:
    BufferView();
    BufferView(void const* src, std::size_t size);
    BufferView(void* src, std::size_t size);
    BufferView(void* src, std::size_t size, std::size_t pos);

    // Get a pointer to the underlying Buffer starting at the current position.
    // Use rewind(), flip(), or clear() first to get the Buffer's origin.
    std::uint8_t const* cbuf() const;

    BufferView view(std::size_t pos, std::size_t size) const;
    BufferView view(std::size_t size) const { return view(pos, pos + size); }
    BufferView view() const { return view(pos, lim); }

    std::uint8_t const& operator[](std::size_t i) const;
    std::uint8_t const& operator*() const;

    std::size_t limit() const noexcept { return lim; }
    std::size_t position() const noexcept { return pos; }
    std::size_t remaining() const noexcept { return lim - pos; }

    virtual BufferView& seek(std::size_t i);
    virtual BufferView& limit(std::size_t limit) noexcept;

    std::size_t copy_to(void* dst, std::size_t size, std::size_t pos) const;
    std::size_t copy_to(void* dst, std::size_t size) const;

    void* get(void* dst, std::size_t size);

    template <typename T>
    T get()
    {
      static_assert(false,
                    "There is no suitable specialization to deserialize "
                    "this type from the buffer.");
    }

    template <typename T>
    BufferView& operator>>(T& dst)
    {
      dst = get<T>();
      return *this;
    }

    bool operator==(BufferView const& rhs) const;
    bool operator!=(BufferView const& rhs) const { return !(*this == rhs); }

  protected:
    // The underlying Buffer; is never owned by BufferView, but may be owned by
    // its derived types.
    std::uint8_t* b{nullptr};
    // The maximum read position (exclusive) in the Buffer.
    std::size_t lim{0};
    // The current read position in the Buffer.
    std::size_t pos{0};
  };
}


class Buffer final : public detail::BufferView
{
public:
  using View = detail::BufferView;

  static Buffer allocate(std::size_t size);
  static Buffer wrap(void* buf, std::size_t size);
  static Buffer copy(void const* buf, std::size_t size);

  // Allocate an underlying Buffer of `capacity` bytes with an initial limit
  // equal to its capacity and a starting position of zero.
  Buffer(std::size_t capacity);
  // Allocate an underlying Buffer with a capacity of `size` bytes and copy
  // `size` bytes from the given Buffer into it starting at its origin.
  Buffer(std::uint8_t const* src, std::size_t size);
  // Create a modifiable view of the given Buffer with a capacity of `size`
  // bytes, a limit equal to its capacity, and a starting position of zero.
  //
  // Ownership of the Buffer's memory is not transferred.
  Buffer(std::uint8_t* buf, std::size_t size);

  Buffer(Buffer&& rhs);
  Buffer& operator=(Buffer&& rhs);

  Buffer(Buffer const& rhs);
  Buffer& operator=(Buffer const& rhs);

  ~Buffer();

  /*! \brief  Get the maximum modifiable capacity in bytes of the underlying
   *          array.
   *
   * This differs from the buffer's limit in that the limit represents the
   * currently filled portion of the Buffer while the capacity is the total
   * size of the buffer.
   * The limit is adjusted by writing to the Buffer or by flipping or clearing
   * it.
   */
  std::size_t capacity() const noexcept { return cap; }

  /*! \brief  Flip the buffer for e.g. reading a just-written value.
   *
   * The limit is set to the current position and the position is set to zero.
   */
  Buffer& flip() noexcept;
  /*! \brief  Clear the buffer by setting the limit to the capacity and the
   * position to zero. */
  Buffer& clear() noexcept;
  Buffer& seek(std::size_t i) override;
  Buffer& limit(std::size_t newlim) noexcept override;

  std::uint8_t* buf();

  std::uint8_t& operator[](std::size_t i);
  std::uint8_t& operator*();

  // Copies the given bytes to this Buffer, replacing its current contents
  // and settings its limit to the size of the copied array.
  Buffer& replace(void const* src, std::size_t size);

  // Copies the given bytes to this Buffer starting at the current
  // position.
  Buffer& put(void const* src, std::size_t size);

  template <typename T>
  Buffer& put(T const& src)
  {
    static_assert(false,
                  "There is no suitable specialization to serialize "
                  "this type to the buffer.");
  }

  template <typename T>
  Buffer& operator<<(T const& src)
  {
    return put(src);
  }

  // Covariant overrides
  template <typename T>
  Buffer& operator>>(T& dst)
  {
    Buffer::View::operator>>(dst);
    return *this;
  }

private:
  // The maximum capacity of the underlying Buffer.
  // This differs from `lim` in that the underlying Buffer may be larger than
  // the contents currently allocated in it and writing to it may extend its
  // limit up to its capacity.
  std::size_t cap;
  // true if we allocated the underlying Buffer and are responsible for
  // deleting it.
  bool owner;

  // The number of bytes able to be written, potentially past the limit if
  // it's been restricted by flip() or limit(size_t).
  // Unused capacity can be reclaimed by clear().
  std::size_t free() const { return cap - pos; }

  // Create a modifiable view of the given Buffer with the specified
  // capacity, limit, and starting position.
  //
  // Ownership of the Buffer's memory is not transferred.
  Buffer(std::uint8_t* buf, std::size_t cap, std::size_t lim, std::size_t pos);
};
}
