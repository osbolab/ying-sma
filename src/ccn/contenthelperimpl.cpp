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
  ++msg.hops;

  for (auto& metadata : msg.metadata) {
    // Break loops
    if (metadata.publisher == node.id)
      continue;

    if (node.interests->interested_in(metadata)) {
      log.d("** I got metadata I'm interested in! **");
    }

    if (!update(metadata, msg.hops))
      return;

    log.d("Content metadata from n(%v)", header.sender);
    log.d("| distance: %v hop(s)", std::uint32_t(msg.hops));
    log.d("| hash: %v", std::string(metadata.hash));
    log.d("| size: %v bytes", metadata.size);
    log.d("| block size: %v bytes", metadata.block_size);
    for (auto& type : metadata.types)
      log.d("| type: %v", type);
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
                               std::uint8_t hops)
{
  auto it = kct.find(metadata.hash);
  if (it != kct.end()) {
    auto& record = it->second;
    if (hops < record.hops) {
      record.hops = hops;
      return true;
    } else
      return false;
  }

  kct.emplace(metadata.hash, MetaRecord{metadata, hops});

  return true;
}

ContentMetadata ContentHelperImpl::create_new(std::vector<ContentType> types,
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
                                  std::move(types),
                                  name,
                                  node.position(),
                                  node.id,
                                  publish_time);

  kct.emplace(hash, MetaRecord{metadata, 0});
  ann_queue.push_back(hash);
  ++to_announce;

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

std::size_t ContentHelperImpl::announce_metadata()
{
  if (clock::now() < next_announce_time)
    return 0;

  std::vector<ContentMetadata> metas;
  for (auto& pair : kct)
    metas.push_back(pair.second.metadata);

  if (not metas.empty()) {
    g_published = clock::now();
    node.post(ContentAnn(std::move(metas)));
  }

  return metas.size();
}

void ContentHelperImpl::request(std::vector<BlockRequestArgs> requests)
{
  std::vector<BlockFragmentRequest> fragments;

  auto req = requests.begin();
  while (req != requests.end()) {
    auto blocks = cache.find(req->hash);
    if (blocks != nullptr) {
      auto it = blocks->find(req->index);
      if (it != blocks->end() && it->second.complete()) {
        req = requests.erase(req);
        continue;
      }
      /*
      auto& gaps = it->second.gaps;
      for (std::size_t i = 0; i < gaps.size() - 1; i += 2)
        fragments.emplace_back(gaps[i], gaps[i + 1] - gaps[i]);
        */
    }
    ++req;
  }

  /*
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

  if (not requests.empty())
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
