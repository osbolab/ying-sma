#pragma once

#include <sma/ccn/contenthelper.hpp>
#include <sma/ccn/contentmetadata.hpp>
#include <sma/ccn/remotecontent.hpp>
#include <sma/ccn/contentcache.hpp>

#include <sma/networkdistance.hpp>
#include <sma/util/hash.hpp>

#include <sma/ccn/ccnfwd.hpp>

#include <sma/chrono.hpp>
#include <sma/io/log>

#include <iosfwd>
#include <unordered_map>


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

  virtual void fetch_block(Hash const& hash, std::size_t index) override;

private:
  using clock = sma::chrono::system_clock;

  //! Alter the Known Content Table by adding or updating the specified entry.
  /*! \return \a true if the entry was added or updated, or \false if it exists
   *          and nothing was changed.
   */
  bool update_kct(ContentMetadata const& metadata, NetworkDistance distance);

  //! The Known Content Table (KCT) of announced content metadata.
  /*! The KCT reflects all the content in the network for which we will forward
   * requests from consumers. Entries are created when we receive content
   * metadata announcements and expire after some time without recurring
   * announcements.
   */
  std::unordered_map<Hash, RemoteContent> kct;

  ContentCache cache;
};
}
