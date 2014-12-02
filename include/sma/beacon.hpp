#pragma once

#include <sma/util/buffer.hpp>
#include <sma/util/serial.hpp>

#include <cstdint>

namespace sma
{
struct NodeId;
struct Message;

struct Beacon {
  TRIVIALLY_SERIALIZABLE(Beacon, body)

  using size_type = std::uint8_t;
  using body_type = Buffer<size_type>;

  /****************************************************************************
   * Serialized Fields
   */
  body_type body;
  /***************************************************************************/

  Beacon() {}

  Beacon(body_type body)
    : body(std::move(body))
  {
  }

  Beacon(Beacon&&) = default;
  Beacon(Beacon const&) = default;

  Beacon& operator=(Beacon&&) = default;
  Beacon& operator=(Beacon const&) = default;
};
}
