#pragma once

#include <sma/nodeid.hpp>
#include <sma/ccn/interest.hpp>

#include <sma/util/serial.hpp>

#include <vector>

namespace sma
{
struct InterestAnn {
  //! One interest from one node, of which an ann may contain several.
  struct Entry {
    TRIVIALLY_SERIALIZABLE(Entry, interest, is_original)
    //! The type of content this node is interested in.
    Interest interest;
    //! \a true if the node sending this ann is the originator of the
    //! interest.
    /* Otherwise \a false if this is a replication.
     * Replications may not be propagated if the network is at equilibrium, and
     * they may not cause any expiring interest to be refreshed. Original
     * interests must always be propagated as they always decrease entropy in
     * the network.
     */
    bool is_original;

    Entry(Interest interest, bool is_original)
      : interest(interest)
      , is_original(is_original)
    {
    }
  };

  TRIVIALLY_SERIALIZABLE(InterestAnn, interested_node, interests)

private:
  using interest_vec = std::vector<Entry>;

public:
  /****************************************************************************
   * Serialized Fields
   */
  NodeId interested_node;
  interest_vec interests;
  /***************************************************************************/

  InterestAnn(NodeId node,
      interest_vec interests = interest_vec())
    : interested_node(node)
    , interests(std::move(interests))
  {
  }

  InterestAnn(InterestAnn&&) = default;
  InterestAnn(InterestAnn const&) = default;

  InterestAnn& operator=(InterestAnn&&) = default;
  InterestAnn& operator=(InterestAnn const&) = default;
};
}
