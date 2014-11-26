#pragma once

#include <sma/messagetype.hpp>
#include <sma/nodeid.hpp>
#include <sma/ccn/remoteinterest.hpp>
#include <sma/serial/vector.hpp>

#include <vector>
#include <limits>
#include <cassert>

namespace sma
{
  struct Message;

struct InterestMessage final {
  static constexpr MessageType TYPE = 1;

  using value_type = RemoteInterest;
  using interest_vector = std::vector<value_type>;
  // This is the type of the vector size as serialized, and so its size
  // defines the maximum number of elements allowed.
  using count_type = std::uint8_t;

  static InterestMessage read(std::uint8_t const* src, std::size_t size);

  /****************************************************************************
   * Serialized Fields - Order matters!
   */
  NodeId sender;
  interest_vector interests;
  /***************************************************************************/

  InterestMessage(NodeId sender)
    : sender(sender)
  {}
  InterestMessage(NodeId sender, value_type interest);
  InterestMessage(NodeId sender, interest_vector interests);

  template <typename Reader>
  InterestMessage(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w) const;

  Message make_message() const;

private:
  using vec_reader = VectorReader<value_type, count_type>;
  using vec_writer = VectorWriter<value_type, count_type>;
};


template <typename Reader>
InterestMessage::InterestMessage(Reader* r)
  : sender(r->template get<decltype(sender)>())
  , interests(vec_reader::read(r))
{
}

template <typename Writer>
void InterestMessage::write_fields(Writer* w) const
{
  assert(interests.size() <= std::numeric_limits<count_type>::max());
  *w << sender;
  *w << vec_writer(&interests);
}
}
