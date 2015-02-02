#pragma once

#include <sma/ccn/interesthelper.hpp>

#include <sma/ccn/ccnfwd.hpp>

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/interestrank.hpp>
#include <sma/ccn/remoteinterest.hpp>

#include <sma/io/log>

#include <map>
#include <unordered_map>

namespace sma
{
//! Manages the storage, announcement, and replication of content interests.
/*! Consumers express interest in content in order to become targets of
 * dissemination from remote, unknown, providers.
 *
 * For provisioned content to reliably reach consumers a majority of nodes
 * should know generally what content are in demand; they can then make
 * intelligent forwarding and caching decisions to facilitate content where it's
 * wanted without completely flooding the network.
 *
 * While it does appear to be a different sort of flooding, the interest data
 * are intended to be extremely light weight relative to the content metadata
 * and should require a minimum of effort to handle.
 */
class InterestHelperImpl : public InterestHelper
{
public:
  //! Construct a helper to manage interests for the given node.
  InterestHelperImpl(CcnNode& node)
    : InterestHelper(node)
  {
  }

  /* Implement InterestHelper */

  void receive(MessageHeader header, InterestAnn msg) override;
  void insert_new(std::vector<ContentType> types) override;
  bool interested_in(ContentMetadata const& metadata) const override;
  bool know_remote(ContentType const& type) const override;

private:
  //! Schedule the future network broadcast of our tables to our neighbors.
  void schedule_announcement(std::chrono::milliseconds delay
                             = std::chrono::milliseconds(5000));
  //! Broadcast the contents of our tables to our neighbors.
  /* In the case that we have many records stored, they may be ordered and
   * culled based on some preference metrics.
   */
  void announce();

  //! Insert a new, or update an existing, interest received from a remote node.
  bool learn_remote_interest(ContentType const& interest);

  //! The Local Interest Table records content this node is interested in.
  /*! These records always have precedence in dissemination as they are the
   * ultimate source of interests in the network.
   */
  std::map<ContentType, InterestRank> lit;

  //! The Remote Interest Table records content other nodes are interested in.
  /*! This table is populated by InterestAnns from other nodes and its
   * entries expire without recurring announcements.
   *
   * An example application of this information is to decide to forward content
   * announcements only for those content in which we have previously seen
   * interest.
   */
  std::unordered_map<ContentType, RemoteInterest> rit;
};
}
