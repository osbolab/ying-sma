#pragma once

#include <sma/util/serial.hpp>

#include <cstdint>

namespace sma
{
struct BeaconResponse {
  EMPTY_SERIALIZABLE(BeaconResponse)

  /****************************************************************************
   * Serialized Fields
   */
  /***************************************************************************/

  BeaconResponse() {}

  BeaconResponse(BeaconResponse const&) = default;
  BeaconResponse& operator=(BeaconResponse const&) = default;
};
}
