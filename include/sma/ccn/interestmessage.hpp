#pragma once

#include <sma/nodeid.hpp>

#include <sma/ccn/remoteinterest.hpp>

#include <sma/util/reader.hpp>

#include <vector>

namespace sma
{
struct Message;

struct InterestMessage {
  using value_type = RemoteInterest;
  using interest_vector = std::vector<value_type>;

  /****************************************************************************
   * Serialized Fields
   */
  NodeId interested;
  interest_vector interests;
  /***************************************************************************/

  InterestMessage(NodeId interested)
    : interested(interested)
  {}

  InterestMessage(NodeId interested, interest_vector interests)
    : interested(interested)
    , interests(std::move(interests))
  {
  }

  InterestMessage(InterestMessage&&) = default;
  InterestMessage(InterestMessage const&) = default;

  InterestMessage& operator=(InterestMessage&&) = default;
  InterestMessage& operator=(InterestMessage const&) = default;

  template <typename...T>
  InterestMessage(Reader<T...>& r)
    : interested(r.template get<decltype(interested)>())
  {
    r >> interests;
  }

  template <typename Writer>
  void write_fields(Writer& w) const
  {
    w << interested;
    w << interests;
  }
};
}
