#pragma once

#include <sma/helper.hpp>
#include <sma/ccn/ccnfwd.hpp>
#include <sma/ccn/contentmetadata.hpp>
#include <sma/ccn/blockrequestargs.hpp>

#include <sma/util/event.hpp>
#include <sma/util/vec2d.hpp>

#include <iosfwd>
#include <cstdlib>
#include <utility>


namespace sma
{

class CacheEntry
{
public:
  virtual ~CacheEntry() {}

  //! Force the entry to not expire from the cache.
  virtual void freeze() = 0;
  //! Allow the entry to expire from the cache.
  virtual void unfreeze() = 0;

  // \return \a true if the entry will not expire from the cache.
  virtual bool frozen() = 0;
};


class ContentHelper : public Helper
{
public:
  ContentHelper(CcnNode& node);

  virtual ~ContentHelper() {}

  //! Receive a content metadata announcement.
  /*! A metadata announcement indicates that a piece of content is available
   * from the publisher named in the metadata.  The ann may be
   * forwarded so the publisher may be an undefined number of hops remote.
   */
  virtual void receive(MessageHeader header, ContentAnn msg) = 0;

  virtual void receive(MessageHeader header, BlockRequest req) = 0;
  virtual void receive(MessageHeader header, BlockResponse resp) = 0;

  virtual ContentMetadata create_new(ContentType const& type,
                                     ContentName const& name,
                                     std::istream& in) = 0;

  //! Send one message to the network containing the metadata of all of the
  //! specified content items.
  virtual void publish_metadata(std::vector<Hash> hashes) = 0;

  //! Send one message to the network containing all of the given block
  //! requests.
  virtual void request_blocks(std::vector<BlockRequestArgs> requests) = 0;

  //! Broadcast completely the data for the specified block.
  // \return An interface that can be used to keep or mark as free the cache
  //         entry for the block data that were broadcast.
  virtual CacheEntry* broadcast_block(Hash hash, std::size_t index) = 0;

  //! Fired when a nonempty set of block requests arrives from the network.
  Event<std::vector<BlockRequestArgs>> on_blocks_requested;
  //! Fired when a previously broadcast request exceeds its Time to Live
  //! argument.
  Event<Hash, std::size_t> on_request_timeout;
  //! Fired when block data arrive and are cached by the content helper.
  // The \a CacheEntry argument provides functions to keep or release the cache
  // entry for the given data.
  Event<Hash, std::size_t, CacheEntry*> on_block_arrived;
};
}

namespace std
{
template <>
struct hash<pair<sma::Hash, size_t>> {
  size_t operator()(pair<sma::Hash, size_t> const& a) const
  {
    return 37 * hash<sma::Hash>()(a.first) + a.second;
  }
};

template <>
struct less<pair<sma::Hash, size_t>> {
  using arg_type = pair<sma::Hash, size_t>;
  bool operator()(arg_type const& lhs, arg_type const& rhs) const
  {
    return (lhs.first < rhs.first) && (lhs.second < rhs.second);
  }
};
}


#if 0
// Example event handler (callback):

class MyClass
{
  // Store a reference if you want access to the node
  CcnNode& node;

public:
  MyClass(CcnNode& node)
    : node(node)
  {
    node.content->on_block_requested
        += std::bind(&MyClass::block_requested_callback, this);

    node.content->on_block_arrived
        += std::bind(&MyClass::block_arrived_callback, this);
  }

  bool block_requested_callback(Hash content_hash,
                                std::size_t block_index,
                                double utility,
                                std::size_t ttl_ms,
                                Vec2d origin_position)
  {
    // do stuff
    bool receive_more_callbacks = true;
    return receive_more_callbacks;
  }

  bool block_arrived_callback(Hash content_hash, std::size_t block_index)
  {
    return true;
  }
};
#endif
