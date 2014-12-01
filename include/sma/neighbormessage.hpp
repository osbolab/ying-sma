#pragma once

#include <sma/util/buffer.hpp>
#include <sma/util/serial.hpp>

#include <cstdint>

namespace sma
{
struct NodeId;
struct Message;

struct NeighborMessage final {
  using size_type = std::uint8_t;
  using body_type = Buffer<size_type>;

  /****************************************************************************
   * Serialized Fields
   */
  body_type body;
  /***************************************************************************/

  NeighborMessage() {}

  NeighborMessage(body_type body)
    : body(std::move(body))
  {
  }

  NeighborMessage(NeighborMessage&&) = default;
  NeighborMessage(NeighborMessage const&) = default;

  NeighborMessage& operator=(NeighborMessage&&) = default;
  NeighborMessage& operator=(NeighborMessage const&) = default;

  DESERIALIZING_CTOR(NeighborMessage)
    : INIT_FIELD(body)
  {
  }

  SERIALIZER()
  {
    PUT_FIELD(body);
  }
};
}
