#pragma once

#include <sma/ccn/contenttype.hpp>

#include <sma/util/serial.hpp>

#include <vector>

namespace sma
{
struct InterestAnn {
  //! One interest from one node, of which an announcement may contain several.
  struct Entry {
    TRIVIALLY_SERIALIZABLE(Entry, type, is_original);
    //! The type of content this node is interested in.
    ContentType type;
    //! \a true if the node sending this announcement is the originator of the
    //! interest.
    /* Otherwise \a false if this is a replication.
     * Replications may not be propagated if the network is at equilibrium, and
     * they may not cause any expiring interest to be refreshed. Original
     * interests must always be propagated as they always decrease entropy in
     * the network.
     */
    bool is_original;
  };

  TRIVIALLY_SERIALIZABLE(InterestAnn, interests)

private:
  using interest_vector = std::vector<Entry>;

public:
  /****************************************************************************
   * Serialized Fields
   */
  bool is_original;
  interest_vector interests;
  /***************************************************************************/

  InterestAnn(interest_vector interests = interest_vector())
    : interests(std::move(interests))
  {
  }

  InterestAnn(InterestAnn&&) = default;
  InterestAnn(InterestAnn const&) = default;

  InterestAnn& operator=(InterestAnn&&) = default;
  InterestAnn& operator=(InterestAnn const&) = default;
};
}
