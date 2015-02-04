#pragma once

#include <sma/ccn/contenthelper.hpp>
#include <sma/ccn/contentmetadata.hpp>
#include <sma/ccn/remotecontent.hpp>
#include <sma/ccn/contentcache.hpp>
#include <sma/ccn/blockrequest.hpp>

#include <sma/networkdistance.hpp>
#include <sma/util/hash.hpp>

#include <sma/ccn/ccnfwd.hpp>

#include <sma/chrono.hpp>
#include <sma/io/log>

#include <iosfwd>
#include <cstdlib>
#include <utility>
#include <unordered_map>


namespace std
{
template <>
struct hash<pair<sma::Hash, size_t>> {
  size_t operator()(pair<sma::Hash, size_t> const& a) const
  {
    return 37 * hash<sma::Hash>()(a.first) + a.second;
  }
};
}

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
public:
  //! Construct a helper to manage the content for the given node.
  ContentHelperImpl(CcnNode& node)
    : ContentHelper(node)
  {
  }

  virtual void receive(MessageHeader header, ContentAnn msg) override;
  virtual void receive(MessageHeader header, BlockRequest req) override;
  virtual void receive(MessageHeader header, BlockResponse resp) override;

  virtual ContentMetadata create_new(ContentType const& type,
                                     ContentName const& name,
                                     std::istream& in) override;

  virtual void publish(Hash const& hash) override;
  virtual bool should_forward(ContentMetadata const& metadata) const override;

  virtual void request_block(Hash const& hash, std::size_t index) override;

private:
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  struct MetaRecord {
    ContentMetadata metadata;
    NetworkDistance distance;
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
  std::unordered_map<Hash, MetaRecord> kct;
  //! Pending Request Table
  std::unordered_map<std::pair<Hash, std::size_t>, PendingRequest> prt;
};
}

