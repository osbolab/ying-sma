#pragma once

#include <sma/util/bufferdest.hpp>
#include <sma/util/buffersource.hpp>

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
  Buffer(BufferDest& buf);

  Buffer(Buffer const& r);
  Buffer& operator=(Buffer const& r);
  Buffer(Buffer&& r);
  Buffer& operator=(Buffer&& r);

  template <typename Reader>
  Buffer(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w) const;

  std::size_t size() const { return sz; }
  std::uint8_t const* cdata() const { return data; }

private:
  std::uint8_t const* data{nullptr};
  std::size_t sz{0};
  std::unique_ptr<std::uint8_t[]> owned;
};


template <typename SizeT>
Buffer<SizeT>::Buffer(std::size_t size)
  : owned(std::make_unique<std::uint8_t[]>(size))
  , sz(size)
{
  assert(sz <= std::numeric_limits<SizeT>::max());
  data = owned.get();
}

template <typename SizeT>
Buffer<SizeT>::Buffer(std::uint8_t const* data, std::size_t size)
  : Buffer(size)
{
  this->data = data;
}
template <typename SizeT>
Buffer<SizeT>::Buffer(BufferDest& buf)
  : Buffer(buf.size())
{
  buf.read(owned.get(), sz);
}

template <typename SizeT>
Buffer<SizeT>::Buffer(Buffer const& r)
  : Buffer(r.sz)
{
  std::memcpy(owned.get(), data, sz);
}
template <typename SizeT>
Buffer<SizeT>& Buffer<SizeT>::operator=(Buffer const& r)
{
  sz = r.sz;
  owned = std::make_unique<std::uint8_t[]>(r.sz);
  std::memcpy(owned.get(), data, sz);
  data = owned.get();
}

template <typename SizeT>
Buffer<SizeT>::Buffer(Buffer&& r)
  : data(r.data)
  , owned(std::move(r.owned))
  , sz(r.sz)
{
  r.data = nullptr;
  r.sz = 0;
}
template <typename SizeT>
Buffer<SizeT>& Buffer<SizeT>::operator=(Buffer&& r)
{
  data = r.data;
  sz = r.sz;
  std::swap(owned, r.owned);

  r.data = nullptr;
  r.sz = 0;

  return *this;
}

template <typename SizeT>
template <typename Reader>
Buffer<SizeT>::Buffer(Reader* r)
  : sz{r->template get<SizeT>()}
{
  owned = std::make_unique<std::uint8_t[]>(sz);
  data = owned.get();
  if (sz != 0)
    r->read(owned.get(), sz);
}

template <typename SizeT>
template <typename Writer>
void Buffer<SizeT>::write_fields(Writer* w) const
{
  *w << SizeT(sz);
  if (sz != 0)
    w->write(data, sz);
}
}
