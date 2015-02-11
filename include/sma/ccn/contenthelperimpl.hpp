#pragma once

#include <sma/ccn/contenthelper.hpp>

#include <sma/ccn/contentmetadata.hpp>

#include <sma/ccn/blockref.hpp>
#include <sma/ccn/blockrequest.hpp>

#include <sma/util/hash.hpp>

#include <sma/util/event.hpp>

#include <sma/chrono.hpp>

#include <vector>
#include <deque>
#include <unordered_map>

#include <chrono>
#include <iosfwd>
#include <cstdlib>
#include <utility>
#include <memory>


namespace sma
{
struct CcnNode;
struct ContentType;
struct ContentName;
struct ContentMetadata;

struct MessageHeader;
struct ContentAnn;
struct BlockRef;
struct BlockRequest;
struct BlockResponse;
struct BlockRequestArgs;

template <typename...>
struct Event;

class ContentCache;

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
public:
  ContentHelperImpl(CcnNode& node);

  void receive(MessageHeader header, ContentAnn msg) override;
  void receive(MessageHeader header, BlockRequest req) override;
  void receive(MessageHeader header, BlockResponse resp) override;

  ContentMetadata create_new(std::vector<ContentType> types,
                             ContentName const& name,
                             std::istream& in) override;

  std::vector<ContentMetadata> metadata() override;
  std::size_t announce_metadata() override;

  void request(std::vector<BlockRequestArgs> requests) override;
  bool broadcast(BlockRef block) override;

  std::size_t freeze(std::vector<BlockRef> blocks) override;
  std::size_t unfreeze(std::vector<BlockRef> blocks) override;

  //! Fired when a nonempty set of block requests arrives from the network.
  Event<NodeId, std::vector<BlockRequestArgs>>& on_blocks_requested() override
  {
    return blocks_requested_event;
  }
  //! Fired when a previously broadcast request exceeds its Time to Live
  //! argument.
  Event<BlockRef>& on_request_timeout() override
  {
    return request_timeout_event;
  }
  //! Fired when block data arrive and are cached by the content helper.
  // The \a CacheEntry argument provides functions to keep or release the cache
  // entry for the given data.
  Event<BlockRef>& on_block_arrived() override { return block_arrived_event; }


private:
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  struct LocalMetadata {
    LocalMetadata(ContentMetadata data);

    void requested();
    void announced();

    ContentMetadata data;
    time_point last_requested;
    time_point next_announce;
  };

  struct RemoteMetadata {
    RemoteMetadata(ContentMetadata data);

    void announced();

    ContentMetadata data;
    time_point next_announce;
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

  bool learn_remote(ContentMetadata const& meta);


  Event<NodeId, std::vector<BlockRequestArgs>> blocks_requested_event;
  Event<BlockRef> request_timeout_event;
  Event<BlockRef> block_arrived_event;

  std::unique_ptr<ContentCache> cache;

  std::deque<Hash> ann_queue;
  std::size_t to_announce;

  // Local Metadata Table
  // Content for which we have all of the data stored permanently.
  std::vector<LocalMetadata> lmt;

  // Remote Metadata Table
  // Content we know of on the network, but for which we don't have a permanent
  // store.
  std::vector<RemoteMetadata> rmt;

  // Pending Request Table
  std::unordered_map<BlockRef, std::pair<time_point, bool>> prt;
};
}
