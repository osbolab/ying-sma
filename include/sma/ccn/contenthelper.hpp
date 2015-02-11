#pragma once

#include <sma/helper.hpp>

#include <vector>

#include <iosfwd>
#include <cstdlib>
#include <utility>


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

  virtual ContentMetadata create_new(std::vector<ContentType> types,
                                     ContentName const& name,
                                     std::istream& in) = 0;

  virtual std::vector<ContentMetadata> metadata() const = 0;
  virtual std::size_t announce_metadata() = 0;

  //! Send one message to the network containing all of the given block
  //! requests.
  virtual void request(std::vector<BlockRequestArgs> requests) = 0;

  //! Broadcast completely the data for the specified block.
  // \return An interface that can be used to keep or mark as free the cache
  //         entry for the block data that were broadcast.
  virtual bool broadcast(BlockRef block) = 0;

  virtual std::size_t freeze(std::vector<BlockRef> blocks) = 0;
  virtual std::size_t unfreeze(std::vector<BlockRef> blocks) = 0;

  //! Fired when a nonempty set of block requests arrives from the network.
  virtual Event<NodeId, std::vector<BlockRequestArgs>>& on_blocks_requested()
      = 0;
  //! Fired when a previously broadcast request exceeds its Time to Live
  //! argument.
  virtual Event<BlockRef>& on_request_timeout() = 0;
  //! Fired when block data arrive and are cached by the content helper.
  // The \a CacheEntry argument provides functions to keep or release the cache
  // entry for the given data.
  virtual Event<BlockRef>& on_block_arrived() = 0;
};
}
