#pragma once

#include "bytes.hh"

#include <cstring>
#include <cstdint>
#include <utility>


namespace sma
{

class ByteView
{
  friend class ByteBuffer;

protected:
  std::uint8_t* b;
  std::size_t cap;
  std::size_t lim;
  std::size_t pos;

  ByteView(std::uint8_t* src, std::size_t cap)
    : b(src)
    , cap(cap)
    , lim(cap)
    , pos(0)
  {
  }
  ByteView(std::uint8_t* src, std::size_t cap, std::size_t lim, std::size_t pos)
    : b(src)
    , cap(cap)
    , lim(lim)
    , pos(pos)
  {
  }

public:
  static ByteView wrap(const std::uint8_t* src, std::size_t len)
  {
    return ByteView(const_cast<std::uint8_t*>(src), len, len, 0);
  }

  const std::uint8_t* cbuf() const { return b + pos; }
  const char* cstr() const { return char_cp(b + pos); }
  std::string copy_string() const { return sma::copy_string(b + pos); }
  std::string copy_string_to_end() const
  {
    return sma::copy_string(b + pos, remaining());
  }
  std::string copy_string(std::size_t len) const
  {
    return sma::copy_string(b + pos, len);
  }

  ByteView cview() { return ByteView(b, cap, lim, pos); }

  const std::uint8_t& operator[](std::size_t i) const { return b[i]; }
  const std::uint8_t& operator*() const { return b[pos]; }

  std::size_t capacity() const { return cap; }
  std::size_t limit() const { return lim; }
  std::size_t position() const { return pos; }
  std::size_t remaining() const { return lim - pos; }

  // clang-format off
  virtual ByteView& seek(std::size_t i) { pos = i; return *this; }
  virtual ByteView& rewind() { pos = 0; return *this; }
  virtual ByteView& flip() { lim = pos; pos = 0; return *this; }
  virtual ByteView& clear() { lim = cap; pos = 0; return *this; }

  std::size_t get(std::uint8_t* dst, std::size_t len);

  template <typename T>
  T get()
  {
    union coerce { T t; std::uint8_t data[sizeof(T)]; };
    coerce c;
    std::memcpy(c.data, b + pos, sizeof(T));
    pos += sizeof(T);
    return c.t;
  }
  // clang-format on
  template <typename T>
  ByteView& operator>>(T& v);

  bool operator==(const ByteView& rhs) const;
  bool operator!=(const ByteView& rhs) const { return !(*this == rhs); }
};


class ByteBuffer final : public ByteView
{
public:
  // clang-format off
  static ByteBuffer wrap(std::uint8_t* buf, std::size_t len)
  { return ByteBuffer(buf, len); }

  static ByteBuffer wrap(char* buf, std::size_t len)
  { return ByteBuffer(uint8_p(buf), len); }

  static ByteBuffer copy(const std::uint8_t* buf, std::size_t len)
  { return ByteBuffer(buf, len);}

  static ByteBuffer copy(const char* buf, std::size_t len)
  { return ByteBuffer(uint8_cp(buf), len); }

  static ByteBuffer allocate(std::size_t len)
  { return ByteBuffer(len); }

  ByteBuffer(ByteBuffer&& rhs)
    : ByteView(rhs.b, rhs.cap, rhs.lim, rhs.pos), owner(rhs.owner)
  {
    rhs.b = nullptr;
    rhs.owner = false;
    rhs.pos = rhs.cap = rhs.lim = 0;
  }
  // clang-format on

  ByteBuffer duplicate() { return ByteBuffer(*this); }
  ByteBuffer view() { return ByteBuffer(b, cap, lim, pos); }

  ByteBuffer& operator=(const ByteBuffer& rhs)
  {
    if (owner)
      delete[] b;
    owner = true;
    b = new std::uint8_t[rhs.cap];
    cap = rhs.cap;
    lim = rhs.lim;
    pos = rhs.pos;
    std::memcpy(b, rhs.b, cap);
    return *this;
  }

  ~ByteBuffer()
  {
    if (owner)
      delete[] b;
  }

  std::uint8_t* buf() { return b + pos; }
  char* str() { return char_p(b + pos); }

  std::uint8_t& operator[](std::size_t i) { return b[i]; }
  std::uint8_t& operator*() { return b[pos]; }

  std::size_t put(const std::uint8_t* src, std::size_t len);
  std::size_t put(const char* src, std::size_t len)
  {
    return put(uint8_cp(src), len);
  }

  template <typename T>
  std::size_t put(const T& t)
  {
    const std::size_t sz = sizeof(T);
    std::memcpy(b + pos, reinterpret_cast<const char*>(&t), sz);
    pos += sz;
    return sz;
  }

  template <typename T>
  ByteBuffer& operator<<(const T& t)
  {
    put(t);
    return *this;
  }

  template <typename T>
  ByteBuffer& operator>>(T& v)
  {
    v = get<T>();
    return *this;
  }

private:
  bool owner;

  // Allocate
  ByteBuffer(std::size_t capacity)
    : ByteView(new std::uint8_t[capacity], capacity)
    , owner(true)
  {
  }
  // Copy
  ByteBuffer(const std::uint8_t* copy, std::size_t len)
    : ByteView(new std::uint8_t[len], len)
    , owner(true)
  {
    std::memcpy(b, copy, len);
  }
  // Duplicate
  ByteBuffer(const ByteBuffer& rhs)
    : ByteView(new std::uint8_t[rhs.cap], rhs.cap, rhs.lim, rhs.pos)
    , owner(true)
  {
    std::memcpy(b, rhs.b, cap);
  }
  // Wrap
  ByteBuffer(std::uint8_t* buf, std::size_t len)
    : ByteView(buf, len)
    , owner(false)
  {
  }
  // View
  ByteBuffer(std::uint8_t* buf,
             std::size_t cap,
             std::size_t lim,
             std::size_t pos)
    : ByteView(buf, cap, lim, pos)
    , owner(false)
  {
  }
};
}
