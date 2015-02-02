#include <sma/ccn/contenthelperimpl.hpp>

#include <sma/context.hpp>
#include <sma/ccn/ccnnode.hpp>

#include <sma/ccn/contentstore.hpp>
#include <sma/ccn/storedcontent.hpp>

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <sma/messageheader.hpp>
#include <sma/ccn/contentann.hpp>

#include <sma/ccn/interesthelper.hpp>

#include <sma/chrono.hpp>

#include <ctime>
#include <chrono>
#include <istream>


using namespace std::literals::chrono_literals;

namespace sma
{
static sma::chrono::system_clock::time_point g_published;

void ContentHelperImpl::receive(MessageHeader header, ContentAnn msg)
{
  auto& metadata = msg.metadata;
  // Break loops
  if (metadata.publisher == node.id)
    return;

  ++msg.distance;

  if (node.interests->interested_in(metadata)) {
    log.d("** I got content I wanted! **");
  }

  if (!update_kct(metadata, msg.distance))
    return;

  log.d("Content descriptor from n(%v)", header.sender);
  log.d("| distance: %v hops", std::uint32_t(msg.distance));
  log.d("| type: %v", metadata.type);
  log.d("| name: %v", metadata.name);
  log.d("| publisher: %v", metadata.publisher);
  log.d("got it");

  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
      clock::now() - g_published);
  log.i("delay: %v ms", time.count());

  if (node.interests->know_remote(metadata.type)) {
    log.d("--> (forward)");
    node.post(msg);
  } else
    log.d("not forwarding");
  log.d("");
}

bool ContentHelperImpl::update_kct(ContentMetadata const& metadata,
                                   NetworkDistance distance)
{
  auto it = kct.find(metadata.hash);
  if (it != kct.end())
    return it->second.saw(distance);

  kct.emplace(metadata.hash,
              RemoteContent(node,
                            static_cast<ContentHelper&>(*this),
                            local_cache,
                            metadata,
                            distance));
  return true;
}

/*
void ContentHelperImpl::publish(Hash hash)
{
  g_published = clock::now();

  node.post(ContentAnn(
      ContentDescriptor(std::move(hash), type, name, node.id)));
}
*/

StoredContent const* ContentHelperImpl::find_data(Hash hash)
{
  return local_cache.find(hash);
}

ContentMetadata ContentHelperImpl::create_new(ContentType type,
                                              ContentName name,
                                              std::istream& in)
{
  auto result = local_cache.store_from(in, 1024);
  auto& hash = result.first;
  auto& data = *(result.second);

  return ContentMetadata(
      hash, type, name, data.size(), data.block_size(), node.id);
}

void ContentHelperImpl::receive(MessageHeader header, BlockRequest req)
{
  log.d("Block request");
}

void ContentHelperImpl::receive(MessageHeader header, BlockResponse resp)
{
  log.d("Block response");
}
}
