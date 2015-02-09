#pragma once

#include <sma/helper.hpp>
#include <sma/ccn/ccnfwd.hpp>
#include <sma/ccn/contentmetadata.hpp>
#include <sma/ccn/blockindex.hpp>
#include <sma/ccn/blockrequestargs.hpp>

#include <sma/nodeid.hpp>
#include <sma/util/event.hpp>
#include <sma/util/vec2d.hpp>

#include <vector>

#include <iosfwd>
#include <cstdlib>
#include <utility>


namespace sma
{
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

  virtual std::vector<ContentMetadata> metadata() const = 0;
  virtual std::size_t publish_metadata() = 0;

  //! Send one message to the network containing all of the given block
  //! requests.
  virtual void request(std::vector<BlockRequestArgs> requests) = 0;

  //! Broadcast completely the data for the specified block.
  // \return An interface that can be used to keep or mark as free the cache
  //         entry for the block data that were broadcast.
  virtual bool broadcast(Hash hash, BlockIndex index) = 0;

  virtual std::size_t freeze(std::vector<std::pair<Hash, BlockIndex>> blocks)
      = 0;
  virtual std::size_t unfreeze(std::vector<std::pair<Hash, BlockIndex>> blocks)
      = 0;

  //! Fired when a nonempty set of block requests arrives from the network.
  Event<NodeId, std::vector<BlockRequestArgs>> on_blocks_requested;
  //! Fired when a previously broadcast request exceeds its Time to Live
  //! argument.
  Event<Hash, BlockIndex> on_request_timeout;
  //! Fired when block data arrive and are cached by the content helper.
  // The \a CacheEntry argument provides functions to keep or release the cache
  // entry for the given data.
  Event<Hash, BlockIndex> on_block_arrived;
};
}

namespace std
{
template <>
struct hash<pair<sma::Hash, sma::BlockIndex>> {
  size_t operator()(pair<sma::Hash, sma::BlockIndex> const& a) const
  {
    return 37 * hash<sma::Hash>()(a.first) + a.second;
  }
};

template <>
struct less<pair<sma::Hash, sma::BlockIndex>> {
  using arg_type = pair<sma::Hash, sma::BlockIndex>;
  bool operator()(arg_type const& lhs, arg_type const& rhs) const
  {
    return (lhs.first < rhs.first) && (lhs.second < rhs.second);
  }
};
}
