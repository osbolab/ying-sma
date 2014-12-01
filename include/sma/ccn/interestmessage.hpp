#pragma once

#include <sma/nodeid.hpp>
#include <sma/ccn/interest.hpp>

#include <sma/util/serial.hpp>

#include <vector>

namespace sma
{
struct Message;

struct InterestMessage {
  TRIVIALLY_SERIALIZABLE(InterestMessage, interested_node, interests)

  using interest_vector = std::vector<Interest>;

  /****************************************************************************
   * Serialized Fields
   */
  NodeId interested_node;
  interest_vector interests;
  /***************************************************************************/

  InterestMessage(NodeId interested_node,
                  interest_vector interests = interest_vector())
    : interested_node(interested_node)
    , interests(std::move(interests))
  {
  }

  InterestMessage(InterestMessage&&) = default;
  InterestMessage(InterestMessage const&) = default;

  InterestMessage& operator=(InterestMessage&&) = default;
  InterestMessage& operator=(InterestMessage const&) = default;
};
}
