#pragma once

#include <sma/nodeid.hpp>
#include <sma/ccn/interest.hpp>

#include <sma/util/serial.hpp>

#include <vector>

namespace sma
{
struct Announcement;

struct InterestAnnouncement {
  TRIVIALLY_SERIALIZABLE(InterestAnnouncement, interested_node, interests)

private:
  using interest_vector = std::vector<Interest>;

public:
  /****************************************************************************
   * Serialized Fields
   */
  NodeId interested_node;
  interest_vector interests;
  /***************************************************************************/

  InterestAnnouncement(NodeId interested_node,
                       interest_vector interests = interest_vector())
    : interested_node(interested_node)
    , interests(std::move(interests))
  {
  }

  InterestAnnouncement(InterestAnnouncement&&) = default;
  InterestAnnouncement(InterestAnnouncement const&) = default;

  InterestAnnouncement& operator=(InterestAnnouncement&&) = default;
  InterestAnnouncement& operator=(InterestAnnouncement const&) = default;
};
}
