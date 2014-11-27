#pragma once

#include <sma/messagetype.hpp>
#include <sma/ccn/remoteinterest.hpp>

#include <sma/serial/vector.hpp>

#include <vector>
#include <limits>
#include <cassert>

namespace sma
{
struct Message;

struct InterestMessage final {
  static constexpr MessageType TYPE = 64;

  using value_type = RemoteInterest;
  using interest_vector = std::vector<value_type>;
  // This is the type of the vector size as serialized, and so its size
  // defines the maximum number of elements allowed.
  using count_type = std::uint8_t;

  /****************************************************************************
   * Serialized Fields
   */
  interest_vector interests;
  /***************************************************************************/

  InterestMessage() = default;
  InterestMessage(interest_vector interests)
    : interests(std::move(interests))
  {
  }

  InterestMessage(InterestMessage&&) = default;
  InterestMessage(InterestMessage const&) = default;

  InterestMessage& operator=(InterestMessage&&) = default;
  InterestMessage& operator=(InterestMessage const&) = default;

  template <typename Reader>
  InterestMessage(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w) const;

private:
  using vec_reader = VectorReader<value_type, count_type>;
  using vec_writer = VectorWriter<value_type, count_type>;
};


template <typename Reader>
InterestMessage::InterestMessage(Reader* r)
  : interests(vec_reader::read(r))
{
}

template <typename Writer>
void InterestMessage::write_fields(Writer* w) const
{
  assert(interests.size() <= std::numeric_limits<count_type>::max());
  *w << vec_writer(&interests);
}
}
