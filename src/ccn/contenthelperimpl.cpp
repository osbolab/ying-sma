#include <sma/ccn/contenthelperimpl.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <sma/ccn/contentcache.hpp>
#include <sma/ccn/blockdata.hpp>

#include <sma/messageheader.hpp>
#include <sma/ccn/contentann.hpp>
#include <sma/ccn/blockresponse.hpp>

#include <sma/ccn/interesthelper.hpp>

#include <sma/async.hpp>

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
  , cache(new ContentCache(node, 1024 * 16))
  , store(new ContentCache(node))
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
    auto cached_block = cache->find(req->block);
    if (cached_block == cache->end())
      cached_block = store->find(req->block);

    if (cached_block != cache->end() && cached_block != store->end()
        && cached_block.complete()) {
      prt.erase(req->block);
      already_have.push_back(std::move(req->block));
      req = requests.erase(req);
      continue;
    }

    auto const new_expiry = clock::now() + req->ttl<millis>();

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

  if (not requests.empty()) {
    log.d("Sending %v requests", requests.size());
    node.post(BlockRequest(std::move(requests)));
  }

  // WARNING: caller must be reentrant if the callback invokes it!
  // For example: the caller enters this function while looping over a
  // collection that it modifies. If the block arrived handler reenters that
  // caller, the inner execution will modify the collection while the outer
  // execution is still iterating, invalidating its iterators.
  for (auto& block : already_have)
    block_arrived_event(block);

  already_in_request = false;
}


void ContentHelperImpl::receive(MessageHeader header, BlockRequest msg)
{
  log.d("Got %v requests", msg.requests.size());
  for (auto const& req : msg.requests) {
    log.d("| block: %v", req.block);
  }
  log.d("");

  if (not msg.requests.empty())
    blocks_requested_event(header.sender, std::move(msg.requests));
}


void ContentHelperImpl::receive(MessageHeader header, BlockResponse resp)
{
  auto ref = BlockRef(resp.hash, resp.index);

  log.d("Got block");
  log.d("| %v", ref);
  log.d("| sender: %v", header.sender);

  bool is_cached = false;
  bool is_stored = false;

  auto cached = cache->find(ref);
  if (cached != cache->end()) {
    is_cached = true;
    log.d("| I already have cached");
  }

  if (store->find(ref) != store->end()) {
    log.d("| I already have stored");
    bool is_stored = true;
  }

  if (not is_cached and not is_stored) {
    auto cached_data = cache->store(
        ref, resp.size, resp.fragments[0].data, resp.fragments[0].size);
    log.d("| Cached %v bytes", cached_data.size());
  }

  auto prt_search = prt.find(ref);
  if (prt_search == prt.end())
    return;

  log.d("| I had a pending request for this");

  auto const& pending = prt_search->second;
  if (pending.keep_on_arrival && not is_stored) {
    auto stored_data = store->store(
        ref, resp.size, resp.fragments[0].data, resp.fragments[0].size);
    log.d("| Stored %v bytes", stored_data.size());
  }

  log.d("");

  block_arrived_event(ref);
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
                                              void const* src,
                                              std::size_t size)
{
  auto hash = cache->store(src, size);

  auto publish_time
      = std::chrono::duration_cast<std::chrono::seconds>(
            chrono::system_clock::now().time_since_epoch()).count();

  auto metadata = ContentMetadata(hash,
                                  size,
                                  ContentCache::block_size,
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
      log.d("I'm providing %v", local.data.types[0]);
      local.announced();
    }
  }

#if 0
    for (auto const& interest : node.interests->all())
      for (auto const& type : metadata.types)
        if (type == interest.type) {
          if (interest.local()) {
            log.d("| I'm interested in this");
            request({BlockRequestArgs(BlockRef(metadata.hash, 0),
                                      1.23,
                                      millis(10000),
                                      node.id,
                                      node.position(),
                                      true)});
          } else {
            log.d("| I know someone interested in this");
          }
        }
#endif

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

  if (not metas.empty())
    node.post(ContentAnn(std::move(metas)));

  return announced;
}


void ContentHelperImpl::receive(MessageHeader header, ContentAnn msg)
{
  for (auto metadata : msg.metadata) {
    // Break loops
    if (metadata.publisher == node.id)
      continue;

    // Count the link this came over
    ++metadata.hops;

    bool const changed = learn_remote(metadata);
    if (changed) {
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
      log.d("| hash: %v", metadata.hash);
      log.d("| size: %v bytes", metadata.size);
      log.d("| block size: %v bytes", metadata.block_size);
      for (auto& type : metadata.types)
        log.d("| type: %v", type);
      log.d("| name: %v", metadata.name);
      log.d("| publisher: %v", metadata.publisher);
      log.d("| publish time: %v ms", metadata.publish_time_ms);
      log.d("| publish location: %v", std::string(metadata.origin));
    }
  }

  auto const time = std::chrono::duration_cast<std::chrono::milliseconds>(
      clock::now() - g_published);
  // log.i("delay: %v ms", time.count());
}


void ContentHelperImpl::check_pending_requests(time_point when)
{
  if (when != time_point()) {
    asynctask(&ContentHelperImpl::check_pending_requests, this, time_point())
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

bool ContentHelperImpl::broadcast(BlockRef ref)
{
  auto block = cache->find(ref);
  if (block == cache->end() or not block.complete())
    return false;

  node.post(
      BlockResponse(ref.hash,
                    ref.index,
                    block.size(),
                    {BlockFragmentResponse(0, block.data(), block.size())}));

  log.d("Sent block");
  log.d("| %v", ref);
  log.d("| size: %v bytes", block.size());
  log.d("");

  return true;
}


std::size_t ContentHelperImpl::freeze(std::vector<BlockRef> blocks)
{
  std::size_t count = 0;
  for (auto const& ref : blocks) {
    auto block = cache->find(ref);
    if (block != cache->end()) {
      block.frozen(true);
      ++count;
    }
  }

  return count;
}


std::size_t ContentHelperImpl::unfreeze(std::vector<BlockRef> blocks)
{
  std::size_t count = 0;
  for (auto const& ref : blocks) {
    auto block = cache->find(ref);
    if (block != cache->end()) {
      block.frozen(false);
      ++count;
    }
  }

  return count;
}
}
