#include <sma/message.hpp>

#include <cstdint>
#include <cassert>
#include <utility>
#include <vector>
#include <ostream>
#include <cstring>


namespace sma
{
// clang-format off
std::ostream& operator<<(std::ostream& os, Message const& m)
{
  os << "Message { type: " << std::uint64_t(m.type())
     << ", data: <" << m.size() << " bytes> }";
  return os;
}
// clang-format on

Message Message::wrap(Type type,
                      Weight weight,
                      std::uint8_t const* data,
                      data_size_type size)
{
  return Message(Header{type, size}, data, weight);
}


Message Message::copy(Type type,
                      Weight weight,
                      std::uint8_t const* data,
                      data_size_type size)
{
  auto owned_data = std::make_unique<std::uint8_t[]>(size);
  std::memcpy(owned_data.get(), data, size);
  return Message(Header{type, size}, std::move(owned_data), weight);
}

Message::Message(Header header, std::uint8_t const* data, Weight weight)
  : header(std::move(header))
  , data(const_cast<std::uint8_t*>(data))
{
}


Message::Message(Header header,
                 std::unique_ptr<std::uint8_t[]> owned_data,
                 Weight weight)
  : header(std::move(header))
  , owned_data(std::move(owned_data))
{
  this->data = owned_data.get();
}

Message::Message(Message const& r)
  : header(r.header)
  , owned_data(std::make_unique<std::uint8_t[]>(r.header.data_size))
{
  std::memcpy(owned_data.get(), r.data, header.data_size);
  data = owned_data.get();
}

Message::Message(Message&& r)
  : header(std::move(r.header))
  , data(r.data)
  , owned_data(std::move(r.owned_data))
{
  r.data = nullptr;
  std::memset((void*) &r.header, 0, sizeof(Header));
}

Message& Message::operator=(Message&& r)
{
  std::swap(header, r.header);
  std::swap(data, r.data);
  std::swap(owned_data, r.owned_data);
  return *this;
}

Message Message::duplicate() const
{
  return Message(*this);
}
}
