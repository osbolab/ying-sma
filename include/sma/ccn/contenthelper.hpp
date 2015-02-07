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

  virtual void freeze() = 0;
  virtual void unfreeze() = 0;
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

  virtual void publish(Hash const& hash) = 0;
  virtual bool should_forward(ContentMetadata const& metadata) const = 0;

  virtual void request_blocks(std::vector<BlockRequestArgs> requests) = 0;
  virtual CacheEntry* broadcast_block(Hash hash, std::size_t index) = 0;

  // Content Hash, Block Index, Utility, TTL ms (deadline), Origin
  Event<std::vector<BlockRequestArgs>> on_blocks_requested;
  Event<Hash, std::size_t> on_request_timeout;
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
