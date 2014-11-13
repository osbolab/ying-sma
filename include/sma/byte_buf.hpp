#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>


namespace sma
{

template <typename T>
struct arrcopy_w {
  T* arr;
  std::size_t len;
};

template <typename T>
static arrcopy_w<T> arrcopy(T* arr, std::size_t len)
{
  return arrcopy_w<T>{arr, len};
}

namespace detail
{

  class byte_view
  {
    friend class byte_buf;

  protected:
    // The underlying buffer; is never owned by byte_view, but may be owned by
    // its derived types.
    std::uint8_t* b{nullptr};
    // The maximum read position (exclusive) in the buffer.
    std::size_t lim{0};
    // The current read position in the buffer.
    std::size_t pos{0};

    // Create an immutable view of the given array with the given bounds.
    // Does not copy or transfer ownership of the array.
    byte_view(void* src, std::size_t len);
    // Create an immutable view of the given array with the given bounds
    // and beginning at the given index.
    // Does not copy or transfer ownership of the array.
    byte_view(void* src, std::size_t len, std::size_t pos);

  public:
    // Create an immutable view of the interval [src, src+len)
    // Does not copy or transfer ownership of the array.
    static byte_view of(const std::uint8_t* src, std::size_t len);

    // Create an immutable view of an empty buffer.
    // All accession methods are invalidated by default.
    byte_view();

    // Get a pointer to the underlying buffer starting at the current position.
    // Use rewind(), flip(), or clear() first to get the buffer's origin.
    const std::uint8_t* cbuf() const { return b + pos; }

    // Get an immutable view of the interval [pos, pos+len) in the buffer
    byte_view view(std::size_t pos, std::size_t len) const;
    // Get an immutable view of the buffer at most `len` bytes long starting
    // from the buffer's current position.
    byte_view view(std::size_t len) const { return view(pos, pos + len); }
    // Get an immutable view of the buffer.
    // The view can be transformed, but its contents cannot be edited.
    byte_view view() const { return view(pos, lim); }

    const std::uint8_t& operator[](std::size_t i) const;
    const std::uint8_t& operator*() const;

    std::size_t limit() const { return lim; }
    std::size_t position() const { return pos; }
    std::size_t remaining() const { return lim - pos; }

    virtual byte_view& seek(std::size_t i);
    virtual byte_view& limit(std::size_t newlim);

    // Copy at most `len` bytes from this buffer, starting at the `pos`th byte,
    // to the given array.
    std::size_t copy_to(void* dst, std::size_t len, std::size_t pos) const;
    std::size_t copy_to(void* dst, std::size_t len) const;

    // Copy exactly `len` bytes starting from the buffer's current position into
    // the given array.
    byte_view& get(void* dst, std::size_t len);

    // Initialize T with value constructor and fill its memory with the next
    // sizeof(T) bytes in the buffer.
    // Using this in its generic form has very little chance of working.
    // A specialization should be provided for deserializing T.
    template <typename T>
    T get();

    // Read a T into the given reference as if by
    //   t = get<T>();
    template <typename T>
    byte_view& operator>>(T& v);

    // Copy bytes from this buffer to the given array, at most copying the
    // length specified by the array wrapper.
    // Use with arrcopy:
    //   view >> arrcopy(myarray, sizeof myarray);
    template <typename T>
    byte_view& operator>>(const arrcopy_w<T>& dst);

    // Perform a bytewise value equality comparison with the given buffer.
    bool operator==(const byte_view& rhs) const;
    bool operator!=(const byte_view& rhs) const { return !(*this == rhs); }
  };

  template <typename T>
  T byte_view::get()
  {
    assert(remaining() >= sizeof(T));
    union coerce {
      T t;
      std::uint8_t data[sizeof(T)];
    };
    coerce c;
    std::memcpy(c.data, b + pos, sizeof(T));
    pos += sizeof(T);
    return c.t;
  }

  template <typename T>
  byte_view& byte_view::operator>>(const arrcopy_w<T>& dst)
  {
    std::size_t sz = dst.len * sizeof(T);
    const std::size_t max = remaining();
    if (sz > max)
      sz = max;
    void* c = static_cast<void*>(dst.arr);
    copy_to(c, sz);
    return *this;
  }
}


class byte_buf final : public detail::byte_view
{
public:
  using view = detail::byte_view;

  static byte_buf allocate(std::size_t len);
  static byte_buf wrap(void* buf, std::size_t len);
  static byte_buf copy(const void* buf, std::size_t len);


  // Allocate an underlying buffer equal in capacity to the given buffer and
  // assign its contents, limit, and position to those of the given buffer.
  byte_buf(const byte_buf& rhs);
  // Cheaply move the given buffer into this one and zero its members.
  byte_buf(byte_buf&& rhs);
  // Allocate a new buffer equal in len to this and copy this buffer's full
  // contents to it.
  byte_buf duplicate();
  // Copies the contents, dimensions, and position of the given buffer.
  byte_buf& operator=(const byte_buf& rhs);
  byte_buf& operator=(byte_buf&& rhs);

  // Deletes the buffer's backing memory if this buffer was instantiated by
  // copying or via allocate().
  ~byte_buf();


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
  byte_buf& flip();
  // Clears the buffer by setting the limit to the capacity and the position
  // to zero.
  byte_buf& clear();
  byte_buf& seek(std::size_t i) override;
  byte_buf& limit(std::size_t newlim) override;

  // Equivalent to byte_view::cbuf, but the underlying buffer is mutable.
  std::uint8_t* buf() { return b + pos; }

  std::uint8_t& operator[](std::size_t i);
  std::uint8_t& operator*();

  // See byte_view::get(void*, size_t)
  byte_buf& get(void* dst, std::size_t len);

  // See byte_view::get<T>()
  template <typename T>
  T get();

  // Copies the given bytes to this buffer, replacing its current contents
  // and settings its limit to the size of the copied array.
  byte_buf& replace(const void* src, std::size_t len);

  // Copies the given bytes to this buffer starting at the current
  // position.
  byte_buf& put(const void* src, std::size_t len);

  // Directly copies the memory of the given reference to the buffer.
  template <typename T>
  byte_buf& put(const T& t);

  // Writes t to the buffer as if by call to put(T).
  template <typename T>
  byte_buf& operator<<(const T& t);

  template <typename T>
  byte_buf& operator<<(const arrcopy_w<T>& src);

  // See byte_view::operator>>(const arrcopy_w<T>&)
  template <typename T>
  byte_buf& operator>>(const arrcopy_w<T>& dst);

  // See byte_view::operator>>(T&)
  template <typename T>
  byte_buf& operator>>(T& v);

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
  byte_buf(std::size_t capacity);
  // Allocate an underlying buffer with a capacity of `len` bytes and copy
  // `len` bytes from the given buffer into it starting at its origin.
  byte_buf(const std::uint8_t* src, std::size_t len);
  // Create a modifiable view of the given buffer with a capacity of `len`
  // bytes, a limit equal to its capacity, and a starting position of zero.
  //
  // Ownership of the buffer's memory is not transferred.
  byte_buf(std::uint8_t* buf, std::size_t len);
  // Create a modifiable view of the given buffer with the specified
  // capacity, limit, and starting position.
  //
  // Ownership of the buffer's memory is not transferred.
  byte_buf(std::uint8_t* buf,
           std::size_t cap,
           std::size_t lim,
           std::size_t pos);
};


template <typename T>
T byte_buf::get()
{
  return byte_view::get<T>();
}

template <typename T>
byte_buf& byte_buf::operator>>(const arrcopy_w<T>& dst)
{
  byte_view::operator>>(dst);
  return *this;
}

template <typename T>
byte_buf& byte_buf::put(const T& t)
{
  const std::size_t sz = sizeof(T);
  assert(free() >= sz);
  auto src = static_cast<const void*>(&t);
  std::memcpy(b + pos, src, sz);
  pos += sz;
  return *this;
}

template <typename T>
byte_buf& byte_buf::operator<<(const arrcopy_w<T>& src)
{
  const std::size_t sz = sizeof(T) * src.len;
  assert(free() >= sz);
  auto vsrc = static_cast<const void*>(src.arr);
  std::memcpy(b + pos, vsrc, sz);
  pos += sz;
  return *this;
}

template <typename T>
byte_buf& byte_buf::operator<<(const T& t)
{
  put(t);
  return *this;
}
}
