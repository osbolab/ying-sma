#pragma once

#include <cstdint>
#include <memory>
#include <limits>
#include <cstring>

namespace sma
{
template <typename SizeT>
struct ByteArray {
  static ByteArray wrap(std::uint8_t const* data, std::size_t size);
  static ByteArray copy(std::uint8_t const* data, std::size_t size);

  ByteArray() {}
  ByteArray(ByteArray&& r);
  ByteArray& operator=(ByteArray&& r);

  template <typename Reader>
  ByteArray(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w) const;

  std::uint8_t const* data{nullptr};
  std::size_t size{0};

private:
  ByteArray(std::uint8_t const* data,
              std::unique_ptr<std::uint8_t[]> owned,
              std::size_t size);

  std::unique_ptr<std::uint8_t[]> owned;
};

template <typename SizeT>
ByteArray<SizeT> ByteArray<SizeT>::wrap(std::uint8_t const* data,
                                            std::size_t size)
{
  assert(size <= std::numeric_limits<SizeT>::max());
  return ByteArray(data, nullptr, size);
}

template <typename SizeT>
ByteArray<SizeT> ByteArray<SizeT>::copy(std::uint8_t const* data,
                                            std::size_t size)
{
  assert(size <= std::numeric_limits<SizeT>::max());
  auto owned = std::make_unique<std::uint8_t[]>(size);
  std::memcpy(owned.get(), data, size);
  return ByteArray(owned.get(), std::move(owned), size);
}

template <typename SizeT>
ByteArray<SizeT>::ByteArray(std::uint8_t const* data,
                                std::unique_ptr<std::uint8_t[]> owned,
                                std::size_t size)
  : data(data)
  , owned(std::move(owned))
  , size(size)
{
}

template <typename SizeT>
ByteArray<SizeT>::ByteArray(ByteArray&& r)
  : data(r.data)
  , owned(std::move(r.owned))
  , size(r.size)
{
}
template <typename SizeT>
ByteArray<SizeT>& ByteArray<SizeT>::operator=(ByteArray&& r)
{
  data = r.data;
  size = r.size;
  std::swap(owned, r.owned);
  return *this;
}

template <typename SizeT>
template <typename Reader>
ByteArray<SizeT>::ByteArray(Reader* r)
  : size{r->template get<SizeT>()}
{
  if (size != 0) {
    owned = std::make_unique<std::uint8_t[]>(size);
    r->read(owned.get(), size);
    data = owned.get();
  }
}

template <typename SizeT>
template <typename Writer>
void ByteArray<SizeT>::write_fields(Writer* w) const
{
  *w << SizeT(size);
  if (size != 0)
    w->write(data, size);
}
}
