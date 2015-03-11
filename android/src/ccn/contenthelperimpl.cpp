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
#include <string>


namespace sma
{
using std::placeholders::_1;

constexpr std::chrono::milliseconds ContentHelperImpl::min_announce_interval;
constexpr std::size_t ContentHelperImpl::fuzz_announce_min_ms;
constexpr std::size_t ContentHelperImpl::fuzz_announce_max_ms;

constexpr std::chrono::seconds ContentHelperImpl::default_initial_ttl;


ContentHelperImpl::ContentHelperImpl(CcnNode& node)
  : ContentHelper(node)
  , cache(new ContentCache(node, 1024 * 1024))
  , store(new ContentCache(node))
  , to_announce(0)
{
  block_arrived_event += std::bind(&ContentHelperImpl::check_content_complete, this, _1);

  if (auto_announce)
    asynctask(&ContentHelperImpl::announce_metadata, this)
        .do_in(min_announce_interval);
}


ContentMetadata ContentHelperImpl::create_new(std::vector<ContentType> types,
                                              ContentName name,
                                              void const* src,
                                              std::size_t size)
{
  auto hash = store->store(src, size);

  auto publish_time_ms = std::chrono::duration_cast<millis>(
                             clock::now().time_since_epoch()).count();

  auto metadata = ContentMetadata(hash,
                                  size,
                                  ContentCache::block_size,
                                  std::move(types),
                                  name,
                                  node.position(),
                                  node.id,
                                  publish_time_ms,
                                  default_initial_ttl);

  lmt.push_back(metadata);
  ann_queue.push_back(hash);
  ++to_announce;

  return metadata;
}


std::size_t ContentHelperImpl::announce_metadata()
{
  if ((lmt.empty() && rmt.empty())) {
    if (auto_announce)
      asynctask(&ContentHelperImpl::announce_metadata, this)
        .do_in(min_announce_interval);
    return 0;
  }

  std::vector<ContentMetadata> will_announce;

  auto const now = clock::now();

  // Local metadata do not expire, but their announcement frequency is limited.
  // We announce them regardless of interests, but our neighbors are discerning.
  for (auto& local : lmt) {
    if (now >= local.next_announce) {
      local.announced();
      will_announce.push_back(local.data);
    }
  }

  // Remotes are not announced if it's been too long since they were received
  // from the original node; every intermediate node decays their TTL so they
  // expire across the entire network at once.
  auto it = rmt.begin();
  while (it != rmt.end()) {
    if (it->data.expired()) {
      it = rmt.erase(it);
    }  else {
      if (node.interests->contains_any(it->data.types)) {
        if (now >= it->next_announce) {
          log.d("Announcing remote metadata about %v", it->data.types[0]);
          will_announce.push_back(it->data);
          it->announced();
        }
      }
      ++it;
    }
  }

  auto const announced = will_announce.size();

  if (!will_announce.empty())
    node.post(ContentAnn(std::move(will_announce)));

  if (auto_announce)
    asynctask(&ContentHelperImpl::announce_metadata, this)
        .do_in(min_announce_interval);

  return announced;
}


void ContentHelperImpl::receive(MessageHeader header, ContentAnn msg)
{
  for (auto& metadata : msg.metadata) {
    // Break loops
    if (metadata.publisher == node.id)
      continue;

    // Count the link this came over
    ++metadata.hops;

    discover(metadata);
  }
}


bool ContentHelperImpl::discover(ContentMetadata meta)
{
  for (auto& existing : rmt) {
    if (existing.data.hash == meta.hash) {
      if (meta.hops < existing.data.hops) {
        existing.data.hops = meta.hops;
        return true;
      }
      return false;
    }
  }

  log.i("Discovered remote meta: %v", std::string(meta.hash));
  rmt.emplace_back(meta);

  if (node.interests->contains_any(meta.types)) {
    interesting_content_event(meta);

    if (auto_fetch) {
      auto_fetch_meta.emplace(meta.hash, meta);
      for (std::size_t i = 0; i < meta.block_count(); ++i) {
        auto ref = BlockRef(meta.hash, i);
        auto stored = store->find(ref);
        if (stored != store->end())
          continue;
        auto cached = cache->find(ref);
        if (cached != cache->end()) {
          store->store(ref, cached.size(), cached.cdata(), cached.size());
          continue;
        }
        for (auto const& existing : auto_fetch_queue) {
          if (existing.hash == meta.hash && existing.index == i)
            continue;
        }
        auto_fetch_queue.emplace_back(meta.hash, i);
      }
      do_auto_fetch();
    }
  }

  return true;
}


void ContentHelperImpl::do_auto_fetch()
{
  if (auto_fetch_queue.empty())
    return;

  // Collect the first n blocks from the auto fetch queue into one request message
  std::vector<BlockRequestArgs> reqs;
  for (std::size_t i = 0; i < std::min(auto_fetch_queue.size(), std::size_t(16)); ++i) {
    auto block = auto_fetch_queue.front();
    // Move the block to the end of the requests
    auto_fetch_queue.pop_front();
    auto_fetch_queue.push_back(block);

    auto it = auto_fetch_meta.find(block.hash);
    assert(it != auto_fetch_meta.end());
    auto const& meta = it->second;
    // Assign a random utility and add the request
    reqs.emplace_back(block,
                      1.0,
                      default_initial_ttl,
                      node.id,
                      node.position(),
                      meta.hops,
                      true,
                      true);
  }

  if (reqs.empty())
    return;

  log.d("Auto-fetching %v interesting blocks (%v more enqueued)",
        reqs.size(),
        auto_fetch_queue.size());

  request(std::move(reqs));

  if (not auto_fetch_queue.empty())
    asynctask(&ContentHelperImpl::do_auto_fetch, this).do_in(std::chrono::milliseconds(100));
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
    bool cancel = false;
    auto stored_block = store->find(req->block);
    if (stored_block != store->end()) {
      cancel = true;
    } else {
      auto cached_block = cache->find(req->block);
      if (cached_block != cache->end()) {
        // If the request was for permanent storage and the block is 
        // already cached then copy it to the store.
        if (req->keep_on_arrival)
          store->store(req->block, cached_block.size(), cached_block.cdata(), cached_block.size());
        cancel = true;
      }
    }

    if (cancel) {
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
      pending.local_only &= req->local_only;
    } else {
      // Add a new pending request to facilitate timeout and block storage.
      prt.emplace(
          req->block,
          PendingRequest{clock::now(), new_expiry, req->keep_on_arrival, req->local_only});
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

  if (msg.requests.empty())
    return;

  blocks_requested_event(header.sender, std::move(msg.requests));

  if (not auto_respond and not auto_forward_requests)
    return;

  std::vector<BlockRequestArgs> to_forward;

  for (auto const& req : msg.requests) {
    // If we can fulfill the request then do so
    if (auto_respond && broadcast(req.block))
      continue;

    if (!auto_forward_requests)
      continue;

    // If we've already requested this block then forward
    // it when it arrives.
    auto prt_search = prt.find(req.block);
    if (prt_search != prt.end()) {
      auto& pending = prt_search->second;
      pending.local_only = false;
      auto new_expiry = clock::now() + req.ttl<millis>();
      if (pending.expiry < new_expiry) {
        pending.expiry = new_expiry;
        check_pending_requests(new_expiry);
      }
    } else {
      // We don't have any pending request for this block; we should
      // forward one if we know a source of the content and are closer to it
      // than the requester is.
      for (auto const& known : rmt) {
        if (known.data.hash == req.block.hash &&
            known.data.hops <= req.hops_from_block) {
          to_forward.emplace_back(req.block,
                                  1.0,
                                  default_initial_ttl,
                                  node.id,
                                  node.position(),
                                  known.data.hops,
                                  true,
                                  true);
        }
      }
    }
  }

  if (!to_forward.empty())
    request(std::move(to_forward));
}


bool ContentHelperImpl::broadcast(BlockRef ref)
{
  auto block = cache->find(ref);
  if (block == cache->end()) {
    block = store->find(ref);
    if (block == store->end())
      return false;
  }

  log.d("Send %v bytes: %v", block.size(), ref);

  node.post(BlockResponse(ref, block.cdata(), block.size()));

  return true;
}


void ContentHelperImpl::receive(MessageHeader header, BlockResponse msg)
{
  // Stop auto-fetching this block if we are
  auto fetching_it = auto_fetch_queue.begin();
  while (fetching_it != auto_fetch_queue.end()) {
    if (*fetching_it == msg.block) {
      auto_fetch_queue.erase(fetching_it);
      break;
    } else
      ++fetching_it;
  }

  auto stored = store->find(msg.block);
  bool is_stored = stored != store->end();

  // Ignore blocks we have permanently stored
  if (is_stored)
    return;

  log.d("Got block from node %v: %v", header.sender, msg.block);

  bool will_rebroadcast = false;

  // Skip the cache if we're expecting to keep all this content's data.
  auto const it = prt.find(msg.block);
  if (it != prt.end()) {
    auto const& pending = it->second;
    if (pending.keep_on_arrival) {
      log.d("| I'll store this permanently");
      store->store(msg.block, msg.size, msg.data, msg.size);
      is_stored = true;
    }

    will_rebroadcast = auto_respond && !pending.local_only;
  }

  // Cache all blocks we come across
  if (!is_stored && (cache->find(msg.block) == cache->end()))
    cache->store(msg.block, msg.size, msg.data, msg.size);

  block_arrived_event(msg.block);

  if (will_rebroadcast) {
    log.d("| I'll rebroadcast it, too");
    broadcast(msg.block);
  }
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
    request_timedout_event(block);
}


std::size_t ContentHelperImpl::frozen(std::vector<BlockRef> const& blocks,
                                      bool enabled)
{
  std::size_t count = 0;
  for (auto const& ref : blocks) {
    auto block = cache->find(ref);
    if (block != cache->end()) {
      block.frozen(enabled);
      ++count;
    }
  }

  return count;
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


bool ContentHelperImpl::check_content_complete(BlockRef ref)
{
  // Check if we have the whole content
  for (auto const& meta : rmt) {
    if (meta.data.hash == ref.hash) {
      if (store->validate_data(meta.data)) {
        log.i("Content %v complete at %v",
              ref.hash,
              std::chrono::duration_cast<millis>(
              clock::now().time_since_epoch()).count());
        content_complete_event(ref.hash);
      }
      return true;
    }
  }
  log.e("Checking if content complete, but don't have any metadata for it");
  return true;
}


void ContentHelperImpl::log_metadata(NodeId sender, ContentMetadata const& meta)
{
  log.d("Content metadata from n(%v)", sender);
  log.d("| distance: %v hop(s)", std::uint32_t(meta.hops));
  log.d("| ttl: %v ms", meta.ttl<millis>().count());
  log.d("| hash: %v", meta.hash);
  log.d("| size: %v bytes", meta.size);
  log.d("| block size: %v bytes", meta.block_size);
  for (auto& type : meta.types)
    log.d("| type: %v", type);
  log.d("| name: %v", meta.name);
  log.d("| publisher: %v", meta.publisher);
  log.d("| publish time: %v ms", meta.publish_time_ms);
  log.d("| publish location: %v", meta.origin);
}

Event<ContentMetadata>& ContentHelperImpl::on_interesting_content()
{
  return interesting_content_event;
}

Event<NodeId, std::vector<BlockRequestArgs>>&
ContentHelperImpl::on_blocks_requested()
{
  return blocks_requested_event;
}

Event<BlockRef>& ContentHelperImpl::on_request_timeout()
{
  return request_timedout_event;
}

Event<BlockRef>& ContentHelperImpl::on_block_arrived()
{
  return block_arrived_event;
}

Event<Hash>& ContentHelperImpl::on_content_complete()
{
  return content_complete_event;
}


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
  auto const min = std::chrono::seconds(1);
  auto const max = std::chrono::seconds(60);

  auto const now = clock::now();
  auto delay = now - last_requested;
  if (delay > max)
    delay = max;
  if (delay < min)
    delay = min;

  next_announce = now + delay;
}


ContentHelperImpl::RemoteMetadata::RemoteMetadata(ContentMetadata data)
  : data(data)
  , next_announce(clock::now())
{
}

void ContentHelperImpl::RemoteMetadata::announced()
{
  next_announce = clock::now() + std::chrono::seconds(1);
}
}
