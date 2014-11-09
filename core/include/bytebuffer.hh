#pragma once

#include "bytes.hh"

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


class ByteView
{
  friend class ByteBuffer;

protected:
  // The underlying buffer; is never owned by ByteView, but may be owned by
  // its derived types.
  std::uint8_t* b;
  // The maximum read position (exclusive) in the buffer.
  std::size_t lim;
  // The current read position in the buffer.
  std::size_t pos;

  // Create an immutable view of the bounded buffer `src` of `len` bytes.
  ByteView(std::uint8_t* src, std::size_t len);
  // Create a view with a bounded underlying buffer of `len` bytes and a read
  // offset `pos`.
  ByteView(std::uint8_t* src, std::size_t len, std::size_t pos);

public:
  // Create an immutable view of the interval [src, src+len)
  static ByteView wrap(const std::uint8_t* src, std::size_t len);

  // Create an immutable empty buffer.
  ByteView();

  // Get a pointer to the underlying buffer starting at the current position.
  // Use rewind(), flip(), or clear() first to get the buffer's origin.
  const std::uint8_t* cbuf() const { return b + pos; }
  // See `std::uint8_t* cbuf() const`
  const char* cstr() const { return char_cp(b + pos); }

  // Get an immutable view of the interval [pos, pos+len) in the buffer
  ByteView view(std::size_t pos, std::size_t len) const;
  // Get an immutable view of the buffer at most `len` bytes long starting
  // from the buffer's current position.
  ByteView view(std::size_t len) const { return view(pos, pos + len); }
  // Get an immutable view of the buffer.
  // The view can be transformed, but its contents cannot be edited.
  ByteView view() const { return view(pos, lim); }

  const std::uint8_t& operator[](std::size_t i) const;
  const std::uint8_t& operator*() const;

  std::size_t limit() const { return lim; }
  std::size_t position() const { return pos; }
  std::size_t remaining() const { return lim - pos; }

  virtual ByteView& seek(std::size_t i);
  virtual ByteView& limit(std::size_t newlim);

  // Copy at most `len` bytes from this buffer to the given array.
  std::size_t copy_to(void* dst, std::size_t pos, std::size_t len) const;
  std::size_t copy_to(void* dst, std::size_t len) const;

  // Copy exactly `len` bytes starting from the buffer's current position into
  // the given array.
  ByteView& get(void* dst, std::size_t len);

  // Initialize T with value constructor and fill its memory with the next
  // sizeof(T) bytes in the buffer.
  // Using this in its generic form has very little chance of working.
  // A specialization should be provided for deserializing T.
  template <typename T>
  T get();

  // Read a T into the given reference as if by
  //   t = get<T>();
  template <typename T>
  ByteView& operator>>(T& v);

  // Copy bytes from this buffer to the given array, at most copying the
  // length specified by the array wrapper.
  // Use with arrcopy:
  //   view >> arrcopy(myarray, sizeof myarray);
  template <typename T>
  ByteView& operator>>(const arrcopy_w<T>& dst);

  // Perform a bytewise value equality comparison with the given buffer.
  bool operator==(const ByteView& rhs) const;
  bool operator!=(const ByteView& rhs) const { return !(*this == rhs); }
};


class ByteBuffer final : public ByteView
{
public:
  static ByteBuffer allocate(std::size_t len);
  static ByteBuffer wrap(std::uint8_t* buf, std::size_t len);
  static ByteBuffer wrap(char* buf, std::size_t len);
  static ByteBuffer copy(const std::uint8_t* buf, std::size_t len);
  static ByteBuffer copy(const char* buf, std::size_t len);


  ByteBuffer(ByteBuffer&& rhs);
  // Allocate a new buffer equal in len to this and copy this buffer's full
  // contents to it.
  ByteBuffer duplicate();
  // Copies the contents, dimensions, and position of the given buffer.
  ByteBuffer& operator=(const ByteBuffer& rhs);

  // Deletes the buffer's backing memory if this buffer was instantiated by
  // copying or via allocate().
  ~ByteBuffer();


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
  ByteBuffer& flip();
  // Clears the buffer by setting the limit to the capacity and the position
  // to zero.
  ByteBuffer& clear();
  ByteBuffer& seek(std::size_t i) override;
  ByteBuffer& limit(std::size_t newlim) override;

  // Equivalent to ByteView::cbuf, but the underlying buffer is mutable.
  std::uint8_t* buf() { return b + pos; }
  char* str() { return char_p(b + pos); }

  std::uint8_t& operator[](std::size_t i);
  std::uint8_t& operator*();

  // See ByteView::get(void*, size_t)
  ByteBuffer& get(void* dst, std::size_t len);

  // See ByteView::get<T>()
  template <typename T>
  T get();

  // Copies the given bytes to this buffer, replacing its current contents
  // and settings its limit to the size of the copied array.
  ByteBuffer& replace(const std::uint8_t* src, std::size_t len);

  // Copies the given bytes to this buffer starting at the current
  // position.
  ByteBuffer& put(const std::uint8_t* src, std::size_t len);

  // Directly copies the memory of the given reference to the buffer.
  template <typename T>
  ByteBuffer& put(const T& t);

  // Writes t to the buffer as if by call to put(T).
  template <typename T>
  ByteBuffer& operator<<(const T& t);

  // See ByteView::operator>>(const arrcopy_w<T>&)
  template <typename T>
  ByteBuffer& operator>>(const arrcopy_w<T>& src);

  // See ByteView::operator>>(T&)
  template <typename T>
  ByteBuffer& operator>>(T& v);

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
  ByteBuffer(std::size_t capacity);
  // Allocate an underlying buffer with a capacity of `len` bytes and copy
  // `len` bytes from the given buffer into it starting at its origin.
  ByteBuffer(const std::uint8_t* src, std::size_t len);
  // Allocate an underlying buffer equal in capacity to the given buffer and
  // assign its contents, limit, and position to those of the given buffer.
  ByteBuffer(const ByteBuffer& rhs);
  // Create a modifiable view of the given buffer with a capacity of `len`
  // bytes, a limit equal to its capacity, and a starting position of zero.
  //
  // Ownership of the buffer's memory is not transferred.
  ByteBuffer(std::uint8_t* buf, std::size_t len);
  // Create a modifiable view of the given buffer with the specified
  // capacity, limit, and starting position.
  //
  // Ownership of the buffer's memory is not transferred.
  ByteBuffer(std::uint8_t* buf,
             std::size_t cap,
             std::size_t lim,
             std::size_t pos);
};


template <typename T>
T ByteView::get()
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
ByteView& ByteView::operator>>(const arrcopy_w<T>& dst)
{
  std::size_t sz = dst.len * sizeof(T);
  const std::size_t max = remaining();
  if (sz > max)
    sz = max;
  void* c = static_cast<void*>(dst.arr);
  copy_to(c, sz);
  return *this;
}


template <typename T>
T ByteBuffer::get()
{
  return ByteView::get<T>();
}

template <typename T>
ByteBuffer& ByteBuffer::operator>>(const arrcopy_w<T>& dst)
{
  ByteView::operator>>(dst);
  return *this;
}

template <typename T>
ByteBuffer& ByteBuffer::put(const T& t)
{
  const std::size_t sz = sizeof(T);
  assert(free() >= sz);
  auto src = static_cast<const void*>(&t);
  std::memcpy(b + pos, src, sz);
  pos += sz;
  return *this;
}

template <typename T>
ByteBuffer& ByteBuffer::operator<<(const T& t)
{
  put(t);
  return *this;
}
}
