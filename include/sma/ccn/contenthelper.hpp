#pragma once

#include <sma/helper.hpp>
#include <sma/ccn/ccnfwd.hpp>
#include <sma/ccn/storedcontent.hpp>
#include <sma/ccn/storedblock.hpp>

#include <sma/util/event.hpp>

#include <iosfwd>

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

  virtual std::pair<bool, StoredContent const*> stored_content(Hash hash) = 0;

  virtual StoredContent const*
  create_new(ContentType type, ContentName name, std::istream& in) = 0;

  //! Submit a request for the given block to the network.
  virtual void start_fetch(Hash content_hash,
                           std::uint32_t block_idx,
                           std::chrono::milliseconds timeout_ms) = 0;

  Event<StoredBlock const*> on_block_arrived;
  Event<Hash, std::uint32_t> on_fetch_timeout;
};
}
