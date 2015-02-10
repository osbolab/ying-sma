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

    auto lit = node.interests->local();
    log.d("");
    log.d("My interest table");
    for (auto& i : lit)
      log.d("| %v (local)", i.type);
    auto rit = node.interests->remote();
    for (auto& i : rit)
      log.d("| %v (%v hops)", i.type, std::uint32_t(i.hops));
    log.d("");

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

    for (auto& interest : node.interests->all())
      for (auto& type : metadata.types)
        if (type == interest.type) {
          if (interest.local()) {
            log.d("| I'm interested in this");
          } else {
            log.d("| I know someone interested in this");
          }
          std::vector<BlockRequestArgs> reqs;
          reqs.emplace_back(
              metadata.hash, 0, 0.0, 10s, node.id, node.position());
          log.d("| Requesting block");
          log.d("|> index: %v", 0);
          log.d("|> ttl: %v ms", reqs[0].ttl_ms);
          log.d("|> origin: %v", std::string(reqs[0].requester_position));
          request(std::move(reqs));
        }

    if (!update(metadata, msg.hops))
      return;
  }


  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
      clock::now() - g_published);
  // log.i("delay: %v ms", time.count());

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

  return metadata;
}

std::size_t ContentHelperImpl::announce_metadata()
{
  if (clock::now() < next_announce_time)
    return 0;

  std::vector<ContentMetadata> metas;
  for (auto& pair : kct)
    if (pair.second.hops == 0
        || node.interests->contains_any(pair.second.metadata.types))
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

  if (not requests.empty())
    node.post(BlockRequest(std::move(requests)));
}

bool ContentHelperImpl::broadcast(Hash hash, BlockIndex index)
{
  auto blocks = cache.find(hash);
  if (blocks != nullptr) {
    auto it = blocks->find(index);
    if (it != blocks->end() && it->second.complete()) {
      auto& block = it->second;
      node.post(
          BlockResponse(hash,
                        index,
                        block.size,
                        {BlockFragmentResponse(0, block.data, block.size)}));
      return true;
    }
  }
  return false;
}

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

  log.d("Got block");
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
