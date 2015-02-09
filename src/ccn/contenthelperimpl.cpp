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

std::vector<ContentMetadata> ContentHelperImpl::metadata() const
{
  std::vector<ContentMetadata> v;
  for (auto& pair : kct)
    v.push_back(pair.second.metadata);
  return v;
}

void ContentHelperImpl::receive(MessageHeader header, ContentAnn msg)
{
  ++msg.distance;

  for (auto& metadata : msg.metadata) {
    // Break loops
    if (metadata.publisher == node.id)
      continue;

    if (node.interests->interested_in(metadata)) {
      log.d("** I got metadata I'm interested in! **");
    }

    if (!update(metadata, msg.distance))
      return;

    log.d("Content metadata from n(%v)", header.sender);
    log.d("| distance: %v hop(s)", std::uint32_t(msg.distance));
    log.d("| hash: %v", std::string(metadata.hash));
    log.d("| size: %v bytes", metadata.size);
    log.d("| block size: %v bytes", metadata.block_size);
    log.d("| type: %v", metadata.type);
    log.d("| name: %v", metadata.name);
    log.d("| publisher: %v", metadata.publisher);
    log.d("| origin: %v", std::string(metadata.origin));
    log.d("| time: %v", metadata.publish_time);
  }

  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
      clock::now() - g_published);
  log.i("delay: %v ms", time.count());

  log.d("");
}

bool ContentHelperImpl::update(ContentMetadata const& metadata,
                               NetworkDistance distance)
{
  auto it = kct.find(metadata.hash);
  if (it != kct.end()) {
    auto& record = it->second;
    if (distance < record.distance) {
      record.distance = distance;
      return true;
    } else
      return false;
  }

  kct.emplace(metadata.hash, MetaRecord{metadata, distance});

  return true;
}

ContentMetadata ContentHelperImpl::create_new(ContentType const& type,
                                              ContentName const& name,
                                              std::istream& in)
{
  std::size_t const block_size = 1024;

  auto stored = cache.load(in, block_size);
  auto& hash = stored.first;
  auto& size = stored.second;

  auto publish_time
      = std::chrono::duration_cast<std::chrono::seconds>(
            chrono::system_clock::now().time_since_epoch()).count();

  auto metadata = ContentMetadata(hash,
                                  size,
                                  block_size,
                                  type,
                                  name,
                                  node.position(),
                                  node.id,
                                  publish_time);

  kct.emplace(hash, MetaRecord{metadata, 0});

  /*
  log.d("Created content");
  log.d("| hash: %v", std::string(hash));
  log.d("| size: %v bytes", size);
  log.d("| block size: %v bytes", block_size);
  log.d("| name: %v", name);
  log.d("| type: %v", type);
  log.d("| publisher: %v", node.id);
  log.d("| origin: %v", std::string(metadata.origin));
  log.d("| time: %v", metadata.publish_time);
  */

  return metadata;
}

std::size_t ContentHelperImpl::publish_metadata()
{
  std::vector<ContentMetadata> metas;

  for (auto& hash : hashes) {
    log.d("Publish content %v", std::string(hash));

    auto metadata_search = kct.find(hash);
    assert(metadata_search != kct.end());
    auto const& metadata = metadata_search->second.metadata;

    assert(cache.validate_data(metadata));

    metas.push_back(metadata);
  }

  if (not metas.empty()) {
    g_published = clock::now();
    node.post(ContentAnn(std::move(metas)));
  }

  return metas.size();
}

void ContentHelperImpl::request(std::vector<BlockRequestArgs> requests)
{
  /*
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

  auto pending = prt.find({hash, index});
  if (pending != prt.end()) {
    auto& old_req = pending->second;
    if (not old_req.add(fragments)) {
      log.d("| Ignoring block request because I already forwarded it");
      return;
    }
  } else {
    prt.emplace(std::make_pair(hash, index), fragments);
    log.d("| New pending request");
  }
  */

  node.post(BlockRequest(std::move(requests)));
}

bool ContentHelperImpl::broadcast(Hash hash, BlockIndex index) { return true; }

std::size_t
ContentHelperImpl::freeze(std::vector<std::pair<Hash, BlockIndex>> blocks)
{
  return blocks.size();
}

std::size_t
ContentHelperImpl::unfreeze(std::vector<std::pair<Hash, BlockIndex>> blocks)
{
  return blocks.size();
}

void ContentHelperImpl::receive(MessageHeader header, BlockRequest msg)
{
  if (not msg.requests.empty())
    on_blocks_requested(header.sender, std::move(msg.requests));
}

void ContentHelperImpl::receive(MessageHeader header, BlockResponse resp)
{
  auto& blocks = cache.find_or_allocate(resp.hash);
  auto it = blocks.find(resp.index);
  if (it == blocks.end())
    it = blocks.emplace(resp.index, BlockData(resp.index, resp.size)).first;
  auto& block = it->second;

  log.d("Block Response");
  log.d("| hash: %v", std::string(resp.hash));
  log.d("| index: %v", resp.index);
  log.d("| sender: %v", header.sender);

  for (auto& fragment : resp.fragments) {
    log.d("| fragment: [%v, %v]",
          fragment.offset,
          fragment.offset + fragment.size - 1);
    block.insert(fragment.offset, fragment.data, fragment.size);
  }

  if (not block.notified and block.complete()) {
    block.notified = true;
    on_block_arrived(resp.hash, block.index);
  }

  log.d("");
}
}
