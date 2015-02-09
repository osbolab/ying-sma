#pragma once

#include <sma/ccn/contenthelper.hpp>
#include <sma/ccn/contentmetadata.hpp>
#include <sma/ccn/remotecontent.hpp>
#include <sma/ccn/contentcache.hpp>
#include <sma/ccn/blockindex.hpp>
#include <sma/ccn/blockrequest.hpp>
#include <sma/ccn/blockrequestargs.hpp>

#include <sma/networkdistance.hpp>
#include <sma/util/hash.hpp>

#include <sma/ccn/ccnfwd.hpp>

#include <sma/chrono.hpp>
#include <sma/io/log>

#include <vector>
#include <deque>
#include <unordered_map>

#include <iosfwd>
#include <cstdlib>
#include <utility>


namespace sma
{
//! Manages the metadata, data, and traffic for content items in the network.
/*! The content helper's responsibilities include publishing, storing,
 * segmenting, caching, and replicating content and its metadata.
 *
 * The helper does not manage interests as, though they do address the content,
 * they do so only by reference and have no relation to its properties or
 * behavior.
 */
class ContentHelperImpl : public ContentHelper
{
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

public:
  //! Construct a helper to manage the content for the given node.
  ContentHelperImpl(CcnNode& node)
    : ContentHelper(node)
    , next_announce_time(clock::now())
    , to_announce(0)
  {
  }

  void receive(MessageHeader header, ContentAnn msg) override;
  void receive(MessageHeader header, BlockRequest req) override;
  void receive(MessageHeader header, BlockResponse resp) override;

  ContentMetadata create_new(std::vector<ContentType> types,
                             ContentName const& name,
                             std::istream& in) override;

  std::vector<ContentMetadata> metadata() const override;
  std::size_t announce_metadata() override;
  void request(std::vector<BlockRequestArgs> requests) override;
  bool broadcast(Hash hash, BlockIndex index) override;
  std::size_t freeze(std::vector<std::pair<Hash, BlockIndex>> blocks) override;
  std::size_t
  unfreeze(std::vector<std::pair<Hash, BlockIndex>> blocks) override;


private:
  struct MetaRecord {
    ContentMetadata metadata;
    std::uint8_t hops;
  };

  struct PendingRequest {
    PendingRequest(std::vector<BlockFragmentRequest> fragments)
      : fragments(std::move(fragments))
    {
    }

    PendingRequest(PendingRequest&&) = default;
    PendingRequest& operator=(PendingRequest&&) = default;

    bool add(std::vector<BlockFragmentRequest> const& fragments)
    {
      return false;
    }

    std::vector<BlockFragmentRequest> fragments;
  };

  bool update(ContentMetadata const& metadata, NetworkDistance distance);

  //! Store actual content data in memory
  ContentCache cache;
  //! Known Content Table - Remote content for which we have metadata
  std::deque<Hash> ann_queue;
  time_point next_announce_time;
  std::size_t to_announce;

  std::unordered_map<Hash, MetaRecord> kct;
  //! Pending Request Table
  std::unordered_map<std::pair<Hash, BlockIndex>, PendingRequest> prt;
};
}
