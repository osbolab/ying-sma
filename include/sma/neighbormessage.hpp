#pragma once

#include <sma/nodeid.hpp>
#include <sma/messagetype.hpp>

#include <sma/util/buffer.hpp>

#include <cstdint>

namespace sma
{
struct Message;

struct NeighborMessage final {
  static constexpr MessageType TYPE = 0;

  using size_type = std::uint8_t;
  using body_type = Buffer<size_type>;

  /****************************************************************************
   * Serialized Fields
   */
  body_type body;
  /***************************************************************************/


  static NeighborMessage read(std::uint8_t const* src, std::size_t size);

  NeighborMessage(body_type body);
  NeighborMessage(NeighborMessage&& r);
  NeighborMessage& operator=(NeighborMessage&& r);

  template <typename Reader>
  NeighborMessage(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w) const;

  Message make_message() const;
};

template <typename Reader>
NeighborMessage::NeighborMessage(Reader* r)
  : body(r->template get<decltype(body)>())
{
}

template <typename Writer>
void NeighborMessage::write_fields(Writer* w) const
{
  *w << body;
}
}
