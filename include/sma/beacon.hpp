#pragma once

#include <sma/util/serial.hpp>

#include <cstdint>

namespace sma
{
struct Beacon {
  TRIVIALLY_SERIALIZABLE(Beacon, is_response)

  /****************************************************************************
   * Serialized Fields
   */
  bool is_response;
  /***************************************************************************/

  Beacon(bool is_response = false)
    : is_response(is_response)
  {
  }

  Beacon(Beacon const&) = default;
  Beacon& operator=(Beacon const&) = default;
};
}
