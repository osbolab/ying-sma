#pragma once

#include <sma/util/reader.hpp>

#include <cstdint>
#include <memory>
#include <limits>
#include <cstring>
#include <cassert>

namespace sma
{
template <typename SizeT>
struct Buffer {
  Buffer() {}
  Buffer(std::size_t size);
  Buffer(std::uint8_t const* data, std::size_t size);

  Buffer(Buffer&& r);
  Buffer(Buffer const& r);
  Buffer& operator=(Buffer&& r);
  Buffer& operator=(Buffer const& r);

  template <typename... T>
  Buffer(Reader<T...>& r);

  template <typename Writer>
  void write_fields(Writer& w) const;

  std::size_t size() const { return sz; }
  std::uint8_t const* cdata() const { return data.get(); }

private:
  std::size_t sz{0};
  std::unique_ptr<std::uint8_t[]> data;
};


template <typename SizeT>
Buffer<SizeT>::Buffer(std::size_t size)
  : data(std::make_unique<std::uint8_t[]>(size))
  , sz(size)
{
  assert(sz <= std::numeric_limits<SizeT>::max());
}

template <typename SizeT>
Buffer<SizeT>::Buffer(std::uint8_t const* src, std::size_t size)
  : Buffer(size)
{
  std::memcpy(data.get(), src, size);
}

template <typename SizeT>
Buffer<SizeT>::Buffer(Buffer const& r)
  : Buffer(r.sz)
{
  std::memcpy(data.get(), r.data.get(), sz);
}
template <typename SizeT>
Buffer<SizeT>& Buffer<SizeT>::operator=(Buffer const& r)
{
  sz = r.sz;
  data = std::make_unique<std::uint8_t[]>(r.sz);
  std::memcpy(data.get(), r.data.get(), sz);
}

template <typename SizeT>
Buffer<SizeT>::Buffer(Buffer&& r)
  : data(std::move(r.data))
  , sz(r.sz)
{
}

template <typename SizeT>
Buffer<SizeT>& Buffer<SizeT>::operator=(Buffer&& r)
{
  sz = r.sz;
  std::swap(data, r.data);

  return *this;
}

template <typename SizeT>
template <typename... T>
Buffer<SizeT>::Buffer(Reader<T...>& r)
  : sz{r.template get<SizeT>()}
{
  data = std::make_unique<std::uint8_t[]>(sz);
  if (sz != 0)
    r.read(data.get(), sz);
}

template <typename SizeT>
template <typename Writer>
void Buffer<SizeT>::write_fields(Writer& w) const
{
  w << SizeT(sz);
  if (sz != 0)
    w.write(data.get(), sz);
}
}
