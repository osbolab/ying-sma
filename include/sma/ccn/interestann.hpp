#pragma once

#include <sma/nodeid.hpp>
#include <sma/ccn/interest.hpp>

#include <sma/util/serial.hpp>

#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>


namespace sma
{
struct InterestAnn {
  /****************************************************************************
   * Serialized Fields
   */
  std::uint8_t count;
  std::uint16_t size;
  std::uint8_t const* data;
  /***************************************************************************/

  InterestAnn(std::size_t count, std::uint8_t const* data, std::size_t size)
    : count(count)
    , size(size)
    , data(new std::uint8_t[size])
  {
    std::memcpy(this->data, data, size);
  }

  DESERIALIZING_CONSTRUCTOR(InterestAnn)
    : INIT_FIELDS(count, size)
    , data(new std::uint8_t[size])
  {
    GET_BYTES(data, size);
  }

  ~InterestAnn() { delete[] data; }

  SERIALIZER()
  {
    PUT_FIELDS(count, size);
    PUT_BYTES(data, size);
  }

  InterestAnn(InterestAnn&&) = default;
  InterestAnn(InterestAnn const&) = default;

  InterestAnn& operator=(InterestAnn&&) = default;
  InterestAnn& operator=(InterestAnn const&) = default;
};
}
