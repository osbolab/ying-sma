#include <sma/ccn/contenthelperimpl.hpp>

#include <sma/context.hpp>
#include <sma/ccn/ccnnode.hpp>

#include <sma/ccn/contentstore.hpp>

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
  auto& descr = msg.descriptor;
  auto& metadata = descr.metadata;
  // Break loops
  if (metadata.publisher == node->id)
    return;

  ++descr.distance;

  if (node->interests->interested_in(descr)) {
    log.d("** I got content I wanted! **");
  }

  if (!update_kct(descr))
    return;

  /*
  log.d("Content descriptor from n(%v)", header.sender);
  log.d("| distance: %v hops", std::uint32_t(descriptor.distance));
  log.d("| type: %v", descriptor.type);
  log.d("| name: %v", descriptor.name);
  log.d("| publisher: %v", descriptor.publisher);
  */
  log.d("got it");

  log.d(metadata.name.size());
  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
      clock::now() - g_published);
  log.i("delay: %v ms", time.count());

  if (node->interests->know_remote(metadata.type)) {
    log.d("--> (forward)");
    node->post(msg);
  } else
    log.d("not forwarding");
  log.d("");
}

bool ContentHelperImpl::update_kct(ContentDescriptor const& descr)
{
  auto try_add = kct.emplace(descr.hash, descr);
  if (try_add.second)
    return true;
  auto& existing = try_add.first->second;
  return existing.update(descr);
}

/*
void ContentHelperImpl::publish(Hash hash)
{
  g_published = clock::now();

  node->post(ContentAnn(
      ContentDescriptor(std::move(hash), type, name, node->id)));
}
*/

StoredContent const* ContentHelperImpl::create_new(ContentType type,
                                                   ContentName name,
                                                   std::istream& in)
{
  auto stored = local_cache->store_from(in, 1024);
}

void ContentHelperImpl::start_fetch(Hash content_hash,
                                    std::uint32_t block_idx,
                                    std::chrono::milliseconds timeout_ms)
{
  auto it = kct.find(content_hash);
  assert(it != kct.end());

  auto& descr = it->second;
  assert(descr.distance > 0);
  assert(block_idx
         < (1 + (descr.metadata.size - 1) / descr.metadata.block_size));

  std::vector<BlockFragmentRequest> fragments;

  auto cached = local_cache->find(content_hash);
  if (cached != nullptr) {
  }

  //node->post(BlockRequest(content_hash, block_idx));
}

void ContentHelperImpl::receive(MessageHeader header, BlockRequest req) {}

void ContentHelperImpl::receive(MessageHeader header, BlockResponse resp) {}


std::pair<bool, StoredContent const*>
ContentHelperImpl::stored_content(Hash hash)
{
}
}
