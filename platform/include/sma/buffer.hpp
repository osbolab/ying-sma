#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>


namespace sma
{
namespace detail
{
  class buffer_view
  {
    friend class buffer;

  protected:
    // The underlying buffer; is never owned by buffer_view, but may be owned by
    // its derived types.
    std::uint8_t* b{nullptr};
    // The maximum read position (exclusive) in the buffer.
    std::size_t lim{0};
    // The current read position in the buffer.
    std::size_t pos{0};

    // Create an immutable view of the given array with the given bounds.
    // Does not copy or transfer ownership of the array.
    buffer_view(void* src, std::size_t len);
    // Create an immutable view of the given array with the given bounds
    // and beginning at the given index.
    // Does not copy or transfer ownership of the array.
    buffer_view(void* src, std::size_t len, std::size_t pos);

  public:
    // Create an immutable view of the interval [src, src+len)
    // Does not copy or transfer ownership of the array.
    static buffer_view of(std::uint8_t const* src, std::size_t len);

    // Create an immutable view of an empty buffer.
    // All accession methods are invalidated by default.
    buffer_view();

    // Get a pointer to the underlying buffer starting at the current position.
    // Use rewind(), flip(), or clear() first to get the buffer's origin.
    std::uint8_t const* cbuf() const { return b + pos; }

    // Get an immutable view of the interval [pos, pos+len) in the buffer
    buffer_view view(std::size_t pos, std::size_t len) const;
    // Get an immutable view of the buffer at most `len` bytes long starting
    // from the buffer's current position.
    buffer_view view(std::size_t len) const { return view(pos, pos + len); }
    // Get an immutable view of the buffer.
    // The view can be transformed, but its contents cannot be edited.
    buffer_view view() const { return view(pos, lim); }

    std::uint8_t const& operator[](std::size_t i) const;
    std::uint8_t const& operator*() const;

    std::size_t limit() const { return lim; }
    std::size_t position() const { return pos; }
    std::size_t remaining() const { return lim - pos; }

    virtual buffer_view& seek(std::size_t i);
    virtual buffer_view& limit(std::size_t limit);

    // Copy at most `len` bytes from this buffer, starting at the `pos`th byte,
    // to the given array.
    std::size_t copy_to(void* dst, std::size_t len, std::size_t pos) const;
    std::size_t copy_to(void* dst, std::size_t len) const;

    // Copy exactly `len` bytes starting from the buffer's current position into
    // the given array.
    void* get(void* dst, std::size_t len);
    std::uint8_t  get_uint8();
    std::uint16_t get_uint16();
    std::uint32_t get_uint32();
    std::uint64_t get_uint64();

    buffer_view& operator>>(std::uint8_t& dst);
    buffer_view& operator>>(std::uint16_t& dst);
    buffer_view& operator>>(std::uint32_t& dst);
    buffer_view& operator>>(std::uint64_t& dst);

    // Perform a bytewise value equality comparison with the given buffer.
    bool operator==(buffer_view const& rhs) const;
    bool operator!=(buffer_view const& rhs) const { return !(*this == rhs); }
  };
}


class buffer final : public detail::buffer_view
{
public:
  using view = detail::buffer_view;

  static buffer allocate(std::size_t len);
  static buffer wrap(void* buf, std::size_t len);
  static buffer copy(void const* buf, std::size_t len);

  // Allocate an underlying buffer equal in capacity to the given buffer and
  // assign its contents, limit, and position to those of the given buffer.
  buffer(buffer const& rhs);
  // Cheaply move the given buffer into this one and zero its members.
  buffer(buffer&& rhs);
  // Allocate a new buffer equal in len to this and copy this buffer's full
  // contents to it.
  buffer duplicate();
  // Copies the contents, dimensions, and position of the given buffer.
  buffer& operator=(buffer const& rhs);
  buffer& operator=(buffer&& rhs);

  // Deletes the buffer's backing memory if this buffer was instantiated by
  // copying or via allocate().
  ~buffer();

  // Get the maximum modifiable capacity in bytes of the underlying buffer.
  // This differs from the buffer's limit in that the limit represents the
  // currently filled portion of the buffer while the capacity is the total
  // size of the buffer.
  // The limit is adjusted by writing to the buffer or by flipping or clearing
  // it.
  std::size_t capacity() const { return cap; }

  // Flips the buffer for e.g. reading a just-written value.
  // The limit is set to the current (not yet written) position and the
  // position set to zero.
  buffer& flip();
  // Clears the buffer by setting the limit to the capacity and the position
  // to zero.
  buffer& clear();
  buffer& seek(std::size_t i) override;
  buffer& limit(std::size_t newlim) override;

  // Equivalent to buffer_view::cbuf, but the underlying buffer is mutable.
  std::uint8_t* buf() { return b + pos; }

  std::uint8_t& operator[](std::size_t i);
  std::uint8_t& operator*();

  // Copies the given bytes to this buffer, replacing its current contents
  // and settings its limit to the size of the copied array.
  buffer& replace(void const* src, std::size_t len);

  // Copies the given bytes to this buffer starting at the current
  // position.
  buffer& put(void const* src, std::size_t len);
  buffer& put_8(std::uint8_t const& src);
  buffer& put_16(std::uint16_t const& src);
  buffer& put_32(std::uint32_t const& src);
  buffer& put_64(std::uint64_t const& src);

  buffer& operator<<(std::uint8_t const& src);
  buffer& operator<<(std::uint16_t const& src);
  buffer& operator<<(std::uint32_t const& src);
  buffer& operator<<(std::uint64_t const& src);

  // Covariant overrides
  buffer& operator>>(std::uint8_t& dst);
  buffer& operator>>(std::uint16_t& dst);
  buffer& operator>>(std::uint32_t& dst);
  buffer& operator>>(std::uint64_t& dst);

private:
  // The maximum capacity of the underlying buffer.
  // This differs from `lim` in that the underlying buffer may be larger than
  // the contents currently allocated in it and writing to it may extend its
  // limit up to its capacity.
  std::size_t cap;
  // true if we allocated the underlying buffer and are responsible for
  // deleting it.
  bool owner;

  // The number of bytes able to be written, potentially past the limit if
  // it's been restricted by flip() or limit(size_t).
  // Unused capacity can be reclaimed by clear().
  std::size_t free() const { return cap - pos; }

  // Allocate an underlying buffer of `capacity` bytes with an initial limit
  // equal to its capacity and a starting position of zero.
  buffer(std::size_t capacity);
  // Allocate an underlying buffer with a capacity of `len` bytes and copy
  // `len` bytes from the given buffer into it starting at its origin.
  buffer(std::uint8_t const* src, std::size_t len);
  // Create a modifiable view of the given buffer with a capacity of `len`
  // bytes, a limit equal to its capacity, and a starting position of zero.
  //
  // Ownership of the buffer's memory is not transferred.
  buffer(std::uint8_t* buf, std::size_t len);
  // Create a modifiable view of the given buffer with the specified
  // capacity, limit, and starting position.
  //
  // Ownership of the buffer's memory is not transferred.
  buffer(std::uint8_t* buf, std::size_t cap, std::size_t lim, std::size_t pos);
};
}
