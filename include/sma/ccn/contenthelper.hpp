#pragma once

#include <sma/helper.hpp>
#include <sma/ccn/ccnfwd.hpp>
#include <sma/ccn/contentmetadata.hpp>

#include <sma/util/event.hpp>
#include <sma/util/vec2d.hpp>

#include <iosfwd>
#include <cstdlib>


namespace sma
{
class ContentHelper : public Helper
{
  friend class RemoteContent;

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

  virtual void request_block(Hash const& hash, std::size_t index) = 0;

  // Content Hash, Block Index, Utility, TTL ms (deadline), Origin
  Event<Hash, std::size_t, double, std::size_t, Vec2d> on_block_requested;
  Event<Hash, std::size_t> on_block_arrived;
  Event<Hash, std::size_t> on_fetch_timeout;
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
