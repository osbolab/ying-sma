#include <sma/ccn/contenthelperimpl.hpp>

#include <sma/context.hpp>
#include <sma/ccn/ccnnode.hpp>

#include <sma/ccn/contentcache.hpp>

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <sma/messageheader.hpp>
#include <sma/ccn/contentann.hpp>
#include <sma/ccn/blockrequest.hpp>
#include <sma/ccn/blockresponse.hpp>

#include <sma/ccn/interesthelper.hpp>

#include <sma/chrono.hpp>

#include <ctime>
#include <chrono>
#include <string>
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
  if (it != kct.end()) {
    auto& remote_content = it->second;
    return remote_content.saw(distance);
  }

  kct.emplace(
      metadata.hash,
      RemoteContent(static_cast<ContentHelper&>(*this), metadata, distance));
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

ContentMetadata ContentHelperImpl::create_new(ContentType const& type,
                                              ContentName const& name,
                                              std::istream& in)
{
  std::size_t const block_size = 1024;

  auto stored = cache.load(in, block_size);
  auto& hash = stored.first;
  auto& size = stored.second;

  return ContentMetadata(hash, type, name, size, block_size, node.id);
}

void ContentHelperImpl::publish(Hash const& hash)
{
  auto it_meta = kct.find(hash);
  assert(it_meta != kct.end());
  auto const& metadata = it_meta->second.metadata();

  assert(cache.validate_data(metadata));

  node.post(ContentAnn(metadata, 0));
}

void ContentHelperImpl::fetch_block(Hash const& hash, std::size_t index)
{
  std::vector<BlockFragmentRequest> fragments;

  auto blocks = cache.find(hash);
  if (blocks != nullptr) {
    auto it = blocks->find(index);
    if (it != blocks->end()) {
      auto& gaps = it->second.gaps;
      for (std::size_t i = 0; i < gaps.size() - 1; i += 2)
        fragments.emplace_back(gaps[i], gaps[i + 1] - gaps[i]);
    }
  }

  node.post(BlockRequest(hash, index, std::move(fragments)));
}

void ContentHelperImpl::receive(MessageHeader header, BlockRequest req)
{
  log.d("Block request");
  log.d("| hash: %v", std::string(req.hash));
  log.d("| index: %v", req.index);
  for (auto& fragment : req.fragments)
    log.d("| fragment: %v - %v", fragment.offset, fragment.size);

  auto blocks = cache.find(req.hash);
  if (blocks != nullptr) {
    log.d("Can fulfill request");
  } else {
    auto it = kct.find(req.hash);
    if (it != kct.end()) {
      log.d("Should forward block request");
    }
  }
}

void ContentHelperImpl::receive(MessageHeader header, BlockResponse resp)
{
  auto& blocks = cache.find_or_allocate(resp.hash);
  auto it = blocks.find(resp.index);
  if (it == blocks.end()) {
    it = blocks.emplace(resp.index, BlockData(resp.index, resp.size_kB * 1024))
             .first;
    log.d("Allocated new block %v of content %v",
          resp.index,
          std::string(resp.hash));
  }
  auto& block = it->second;

  log.d("Populating block %v of content %v (from node %v)",
        resp.index,
        std::string(resp.hash),
        header.sender);

  for (auto& fragment : resp.fragments) {
    block.insert(fragment.offset, fragment.data, fragment.size);
    log.d("- inserted %v bytes at position %v", fragment.size, fragment.offset);
  }
}
}
