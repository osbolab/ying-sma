#include <sma/ccn/contenthelperimpl.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/ccn/contentcache.hpp>

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <sma/messageheader.hpp>
#include <sma/ccn/contentann.hpp>
#include <sma/ccn/blockresponse.hpp>

#include <sma/ccn/interesthelper.hpp>

#include <ctime>
#include <chrono>
#include <string>
#include <istream>


namespace sma
{
using namespace std::literals::chrono_literals;

static sma::chrono::system_clock::time_point g_published;


ContentHelperImpl::LocalMetadata::LocalMetadata(ContentMetadata data)
  : data(data)
  , last_requested(clock::now())
  , next_announce(clock::now())
{
}


void ContentHelperImpl::LocalMetadata::requested()
{
  last_requested = clock::now();
}


void ContentHelperImpl::LocalMetadata::announced()
{
  // 2^t growth
  auto const now = clock::now();
  auto delay = now - last_requested;
  if (delay > 60s)
    delay = 60s;
  if (delay < 1s)
    delay = 1s;

  next_announce = now + delay;
}


ContentHelperImpl::RemoteMetadata::RemoteMetadata(ContentMetadata data)
  : data(data)
  , next_announce(clock::now())
{
}

void ContentHelperImpl::RemoteMetadata::announced()
{
  next_announce = clock::now() + 1s;
}



ContentHelperImpl::ContentHelperImpl(CcnNode& node)
  : ContentHelper(node)
  , cache(new ContentCache())
  , to_announce(0)
{
}


std::vector<ContentMetadata> ContentHelperImpl::metadata()
{
  std::vector<ContentMetadata> v;
  v.reserve(lmt.size() + rmt.size());

  for (auto const& local : lmt)
    v.push_back(local.data);

  auto it = rmt.begin();
  while (it != rmt.end())
    if (it->data.expired())
      it = rmt.erase(it);
    else
      v.push_back((it++)->data);

  v.shrink_to_fit();
  return v;
}


void ContentHelperImpl::receive(MessageHeader header, ContentAnn msg)
{
  for (auto metadata : msg.metadata) {
    // Break loops
    if (metadata.publisher == node.id)
      continue;

    // Count the link this came over
    ++metadata.hops;

    auto const lit = node.interests->local();
    log.d("");
    log.d("My interest table");
    for (auto const& i : lit)
      log.d("| %v (local)", i.type);
    auto rit = node.interests->remote();
    for (auto const& i : rit)
      log.d("| %v (%v hops)", i.type, std::uint32_t(i.hops));
    log.d("");

    log.d("Content metadata from n(%v)", header.sender);
    log.d("| distance: %v hop(s)", std::uint32_t(metadata.hops));
    log.d("| ttl: %v ms", metadata.ttl<std::chrono::milliseconds>().count());
    log.d("| hash: %v", std::string(metadata.hash));
    log.d("| size: %v bytes", metadata.size);
    log.d("| block size: %v bytes", metadata.block_size);
    for (auto& type : metadata.types)
      log.d("| type: %v", type);
    log.d("| name: %v", metadata.name);
    log.d("| publisher: %v", metadata.publisher);
    log.d("| publish time: %v ms", metadata.publish_time_ms);
    log.d("| publish location: %v", std::string(metadata.origin));

    bool const changed = learn_remote(metadata);
    log.d("| learned something new: %v", (changed ? "yes" : "no"));

    for (auto const& interest : node.interests->all())
      for (auto const& type : metadata.types)
        if (type == interest.type) {
          if (interest.local()) {
            log.d("| I'm interested in this");
          } else {
            log.d("| I know someone interested in this");
          }
        }
  }

  auto const time = std::chrono::duration_cast<std::chrono::milliseconds>(
      clock::now() - g_published);
  // log.i("delay: %v ms", time.count());

  log.d("");
}


void ContentHelperImpl::receive(MessageHeader header, BlockRequest msg)
{
  if (not msg.requests.empty())
    blocks_requested_event(header.sender, std::move(msg.requests));
}


void ContentHelperImpl::receive(MessageHeader header, BlockResponse resp)
{
  auto& blocks = cache->find_or_allocate(resp.hash);
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
    block_arrived_event(BlockRef(resp.hash, block.index));
  }

  log.d("");
}


bool ContentHelperImpl::learn_remote(ContentMetadata const& meta)
{
  for (auto& existing : rmt)
    if (existing.data.hash == meta.hash) {
      if (meta.hops < existing.data.hops) {
        existing.data.hops = meta.hops;
        return true;
      } else
        return false;
    }

  rmt.emplace_back(meta);
  return true;
}


ContentMetadata ContentHelperImpl::create_new(std::vector<ContentType> types,
                                              ContentName const& name,
                                              std::istream& in)
{
  std::size_t const block_size = 1024;

  auto stored = cache->load(in, block_size);
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
                                  publish_time,
                                  10s);

  lmt.push_back(metadata);
  ann_queue.push_back(hash);
  ++to_announce;

  return metadata;
}


std::size_t ContentHelperImpl::announce_metadata()
{
  if ((lmt.empty() && rmt.empty()))
    return 0;

  std::vector<ContentMetadata> metas;
  metas.reserve(lmt.size() + rmt.size());

  auto const now = clock::now();

  // Local metadata do not expire, but their announcement frequency is limited.
  for (auto& local : lmt) {
    if (now >= local.next_announce) {
      metas.push_back(local.data);
      local.announced();
    }
  }

  // Remotes are not announced if it's been too long since they were received
  // from the original node; every intermediate node decays their TTL so they
  // expire across the entire network at once.
  auto it = rmt.begin();
  while (it != rmt.end())
    if (it->data.expired())
      it = rmt.erase(it);
    else if (node.interests->contains_any(it->data.types)) {
      metas.push_back(it->data);
      it->announced();
      ++it;
    }

  metas.shrink_to_fit();

  auto const announced = metas.size();

  if (not metas.empty()) {
    g_published = clock::now();
    node.post(ContentAnn(std::move(metas)));
  }

  return announced;
}


void ContentHelperImpl::request(std::vector<BlockRequestArgs> requests)
{
  // Detect reentrance
  if (already_in_request)
    log.w("ContentHelperImpl::request has been reentered. Are you SURE the "
          "caller is reentrant?");
  already_in_request = true;

  std::vector<BlockRef> already_have;

  auto req = requests.begin();
  while (req != requests.end()) {
    // Skip a request if we have the block in cache or store.
    auto cached_block = cache.find(req->block);
    if (cached_block == nullptr)
      cached_block = store.find(req->block);

    if (cached_block != nullptr && cached_block->complete()) {
      prt.erase(req->block);
      already_have.push_back(std::move(*req));
      req = requests.erase(req);
      continue;
    }

    auto const new_expiry = clock::now() + req->ttl();

    auto it = prt.find(req->block);
    if (it != prt.end()) {
      auto& pending = it->second;
      // Update existing pending requests to have a longer TTL or keep the
      // requested block.
      if (pending.expiry < new_expiry) {
        pending.expiry = new_expiry;
        check_pending_requests(new_expiry);
      }
      pending.keep_on_arrival |= req->keep_on_arrival;
    } else {
      // Add a new pending request to facilitate timeout and block storage.
      prt.emplace(
          req->block,
          PendingRequest{clock::now(), new_expiry, req->keep_on_arrival});
      check_pending_requests(new_expiry);
    }

    ++req;
  }

  if (not requests.empty())
    node.post(BlockRequest(std::move(requests)));

  // WARNING: caller must be reentrant if the callback invokes it!
  // For example: the caller enters this function while looping over a
  // collection that it modifies. If the block arrived handler reenters that
  // caller, the inner execution will modify the collection while the outer
  // execution is still iterating, invalidating its iterators.
  for (auto& block : already_have)
    block_arrived_event(block);

  already_in_request = false;
}


void ContentHelperImpl::check_pending_requests(time_point when)
{
  if (when != time_point()) {
    asynctask(&ContentHelperImpl::check_pending_requests, this)
        .do_in(when - clock::now());
    return;
  }

  std::vector<BlockRef> timed_out;

  auto const now = clock::now();
  auto it = prt.begin();
  while (it != prt.end()) {
    auto const& pending = it->second;
    if (now >= pending.expiry) {
      timed_out.push_back(std::move(it->first));
      it = prt.erase(it);
    } else
      ++it;
  }

  for (auto& block : timed_out)
    request_timeout_event(block);
}


bool ContentHelperImpl::broadcast(BlockRef block)
{
  auto existing = cache->find(block);
  if (existing != nullptr) {
  }

  auto blocks = cache->find(block.hash);
  if (blocks != nullptr) {
    auto it = blocks->find(block.index);
    if (it != blocks->end() && it->second.complete()) {
      auto& found = it->second;
      node.post(
          BlockResponse(block.hash,
                        block.index,
                        found.size,
                        {BlockFragmentResponse(0, found.data, found.size)}));
      return true;
    }
  }
  return false;
}


std::size_t ContentHelperImpl::freeze(std::vector<BlockRef> blocks)
{
  return blocks.size();
}


std::size_t ContentHelperImpl::unfreeze(std::vector<BlockRef> blocks)
{
  return blocks.size();
}
}
