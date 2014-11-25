#pragma once

#include <cstdint>
#include <memory>
#include <limits>
#include <cstring>

namespace sma
{
template <typename SizeT>
struct MessageBody {
  static MessageBody wrap(std::uint8_t const* data, std::size_t size);
  static MessageBody copy(std::uint8_t const* data, std::size_t size);

  MessageBody() {}
  MessageBody(MessageBody&& r);
  MessageBody& operator=(MessageBody&& r);

  template <typename Reader>
  MessageBody(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w) const;

  std::uint8_t const* data{nullptr};
  std::size_t size{0};

private:
  MessageBody(std::uint8_t const* data,
              std::unique_ptr<std::uint8_t[]> owned,
              std::size_t size);

  std::unique_ptr<std::uint8_t[]> owned;
};

template <typename SizeT>
MessageBody<SizeT> MessageBody<SizeT>::wrap(std::uint8_t const* data,
                                            std::size_t size)
{
  assert(size <= std::numeric_limits<SizeT>::max());
  return MessageBody(data, nullptr, size);
}

template <typename SizeT>
MessageBody<SizeT> MessageBody<SizeT>::copy(std::uint8_t const* data,
                                            std::size_t size)
{
  assert(size <= std::numeric_limits<SizeT>::max());
  auto owned = std::make_unique<std::uint8_t[]>(size);
  std::memcpy(owned.get(), data, size);
  return MessageBody(owned.get(), std::move(owned), size);
}

template <typename SizeT>
MessageBody<SizeT>::MessageBody(std::uint8_t const* data,
                                std::unique_ptr<std::uint8_t[]> owned,
                                std::size_t size)
  : data(data)
  , owned(std::move(owned))
  , size(size)
{
}

template <typename SizeT>
MessageBody<SizeT>::MessageBody(MessageBody&& r)
  : data(r.data)
  , owned(std::move(r.owned))
  , size(r.size)
{
}
template <typename SizeT>
MessageBody<SizeT>& MessageBody<SizeT>::operator=(MessageBody&& r)
{
  data = r.data;
  size = r.size;
  std::swap(owned, r.owned);
  return *this;
}

template <typename SizeT>
template <typename Reader>
MessageBody<SizeT>::MessageBody(Reader* r)
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
void MessageBody<SizeT>::write_fields(Writer* w) const
{
  *w << SizeT(size);
  if (size != 0)
    w->write(data, size);
}
}
