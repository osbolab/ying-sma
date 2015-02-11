#pragma once

#include <sma/ccn/contenthelper.hpp>

#include <sma/ccn/blockrequest.hpp>
#include <sma/ccn/contentcache.hpp>

#include <sma/util/hash.hpp>

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
  bool broadcast(BlockRef block) override;

  std::size_t freeze(std::vector<BlockRef> blocks) override;
  std::size_t unfreeze(std::vector<BlockRef> blocks) override;

  //! Fired when a nonempty set of block requests arrives from the network.
  Event<NodeId, std::vector<BlockRequestArgs>>& on_blocks_requested() override;
  //! Fired when a previously broadcast request exceeds its Time to Live
  //! argument.
  Event<BlockRef>& on_request_timeout() override;
  //! Fired when block data arrive and are cached by the content helper.
  // The \a CacheEntry argument provides functions to keep or release the cache
  // entry for the given data.
  Event<BlockRef>& on_block_arrived() override;


private:
  struct LocalMetadata {
    ContentMetadata data;
    time_point last_requested;
    time_point last_announced;
  };

  struct RemoteMetadata {
    ContentMetadata data;
    std::size_t hops;
    time_point expiry_time;
  };

  /// Track the requested segments of each block additively; two requests for
  /// different fragments of the same block become a single request for the
  /// union of those two fragments.
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

  // Blocks we are storing transiently because we received them and may be asked
  // to broadcast them in the near future.
  // Acts as a least-recently-requested cache by default, but elements can be
  // locked at the head on request.
  ContentCache cache;

  std::deque<Hash> ann_queue;
  time_point next_announce_time;
  std::size_t to_announce;

  // Local Metadata Table
  // Content for which we have all of the data stored permanently.
  std::vector<LocalMetadata> lmt;

  // Remote Metadata Table
  // Content we know of on the network, but for which we don't have a permanent
  // store.
  std::vector<RemoteMetadata> rmt;

  // Pending Request Table
  // Content blocks that have been requested from us.
  std::unordered_map<BlockRef, time_point> prt;
};
}
