#pragma once

#include <sma/messagetype.hpp>

#include <sma/util/buffer.hpp>

#include <cstdint>

namespace sma
{
struct NodeId;
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

  NeighborMessage(body_type body)
    : body(std::move(body))
  {}

  NeighborMessage(NeighborMessage&&) = default;
  NeighborMessage(NeighborMessage const&) = default;

  NeighborMessage& operator=(NeighborMessage&&) = default;
  NeighborMessage& operator=(NeighborMessage const&) = default;

  template <typename Reader>
  NeighborMessage(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w) const;
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
