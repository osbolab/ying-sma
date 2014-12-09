#pragma once

#include <sma/util/serial.hpp>

#include <cstdint>

namespace sma
{
struct Beacon {
  EMPTY_SERIALIZABLE(Beacon)

  /****************************************************************************
   * Serialized Fields
   */
  /***************************************************************************/

  Beacon() {}

  Beacon(Beacon const&) = default;
  Beacon& operator=(Beacon const&) = default;
};
}
