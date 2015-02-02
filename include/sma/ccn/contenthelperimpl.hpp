#pragma once

#include <sma/ccn/contenthelper.hpp>

#include <sma/ccn/contentdescriptor.hpp>
#include <sma/util/hash.hpp>

#include <sma/ccn/ccnfwd.hpp>

#include <sma/chrono.hpp>
#include <sma/io/log>

#include <iosfwd>
#include <unordered_map>

namespace sma
{
class ContentStore;

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
  ContentHelperImpl(CcnNode& node, ContentStore* local_cache)
    : ContentHelper(node)
    , local_cache(local_cache)
  {
  }

  /* Implement ContentHelper */

  void receive(MessageHeader header, ContentAnn msg) override;

  void receive(MessageHeader header, BlockRequest req) override;
  void receive(MessageHeader header, BlockResponse resp) override;


  std::pair<bool, StoredContent const*> stored_content(Hash hash) override;

  StoredContent const*
  create_new(ContentType type, ContentName name, std::istream& in) override;

  void start_fetch(Hash content_hash,
                   std::uint32_t block_idx,
                   std::chrono::milliseconds timeout_ms) override;


private:
  using clock = sma::chrono::system_clock;

  //! Alter the Known Content Table by adding or updating the specified entry.
  /*! \return \a true if the entry was added or updated, or \false if it exists
   *          and nothing was changed.
   */
  bool update_kct(ContentDescriptor const& descriptor);

  //! The Known Content Table (KCT) of announced content metadata.
  /*! The KCT reflects all the content in the network for which we will forward
   * requests from consumers. Entries are created when we receive content
   * metadata announcements and expire after some time without recurring
   * announcements.
   */
  std::unordered_map<Hash, ContentDescriptor> kct;

  ContentStore* local_cache;
};
}
