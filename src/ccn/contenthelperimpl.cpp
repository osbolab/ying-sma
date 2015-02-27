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
constexpr std::chrono::milliseconds ContentHelperImpl::min_announce_interval;
constexpr std::size_t ContentHelperImpl::fuzz_announce_min_ms;
constexpr std::size_t ContentHelperImpl::fuzz_announce_max_ms;

constexpr std::chrono::milliseconds ContentHelperImpl::default_initial_ttl;


ContentHelperImpl::ContentHelperImpl(CcnNode& node)
  : ContentHelper(node)
  , cache(new ContentCache(node, 64 * 1024, false))
  , store(new ContentCache(node))
  , to_announce(0)
{
  if (auto_announce)
    asynctask(&ContentHelperImpl::announce_metadata, this)
        .do_in(min_announce_interval);
}


ContentMetadata ContentHelperImpl::create_new(std::vector<ContentType> types,
                                              ContentName name,
                                              void const* src,
                                              std::size_t size)
{
//  auto hash = cache->store(src, size);
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


std::uint16_t ContentHelperImpl::announce_metadata()
{
  if ((lmt.empty() && rmt.empty()))
    return 0;

  std::vector<ContentMetadata> will_announce;
  will_announce.reserve(lmt.size() + rmt.size());

  auto const now = clock::now();

  // Local metadata do not expire, but their announcement frequency is limited.
  // We announce them regardless of interests, but our neighbors are discerning.
  for (auto& local : lmt) {
    if (now >= local.next_announce) {
      will_announce.push_back(local.data);
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
    else {
      if (node.interests->contains_any(it->data.types)) {
        will_announce.push_back(it->data);
        it->announced();
      }
      ++it;
    }

  will_announce.shrink_to_fit();

//  auto const announced = will_announce.size();

  std::size_t max_announce = 1;

  std::uint16_t bytes_sent = 0;

  if (not will_announce.empty()) {
    if (will_announce.size() > max_announce) {
      std::vector<ContentMetadata> actual_announce;
      while (max_announce > 0) {
        int ix = rand() % will_announce.size();
        auto meta = will_announce[ix];
        will_announce.erase (will_announce.begin() + ix);
        actual_announce.push_back (meta);
        max_announce--;
      }
      bytes_sent = node.post(ContentAnn(std::move(actual_announce)));
    } else {
      bytes_sent = node.post(ContentAnn(std::move(will_announce)));
    }
  }

//  if (auto_announce)
//    asynctask(&ContentHelperImpl::announce_metadata, this)
//        .do_in(min_announce_interval);

  return bytes_sent;

//  return announced;
}


void ContentHelperImpl::receive(MessageHeader header, ContentAnn msg)
{
  for (auto metadata : msg.metadata) {
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
  for (auto& existing : rmt)
    if (existing.data.hash == meta.hash) {
      if (meta.hops < existing.data.hops) {
        existing.data.hops = meta.hops;
        return true;
      } else
        return false;
    }

  rmt.emplace_back(meta);

  if (node.interests->contains_any(meta.types)) {
    interesting_content_event(meta);

    if (auto_fetch) {
      for (std::size_t i = 0; i < meta.block_count(); ++i)
        auto_fetch_queue.emplace_back(meta.hash, i);
      do_auto_fetch();
    }
  }

  return true;
}


void ContentHelperImpl::do_auto_fetch()
{
  if (auto_fetch_queue.empty())
    return;

  std::vector<BlockRequestArgs> reqs;

  for (std::size_t i = 0; i < 4 && !auto_fetch_queue.empty(); ++i) {
    auto block = auto_fetch_queue.front();
    auto it = rmt.begin();
    for (auto const& meta : rmt)
      if (meta.data.hash == block.hash) {
        reqs.emplace_back(block,
                          1.0,
                          default_initial_ttl,
                          node.id,
                          node.position(),
                          meta.data.hops,
                          true,
                          true);
        break;
      }

    auto_fetch_queue.pop_front();
  }

  log.d("Auto-fetching %v interesting blocks (%v more enqueued)",
        reqs.size(),
        auto_fetch_queue.size());

  request(std::move(reqs));

  if (not auto_fetch_queue.empty())
    asynctask(&ContentHelperImpl::do_auto_fetch, this).do_in(std::chrono::seconds(1));
}


std::uint16_t ContentHelperImpl::request(std::vector<BlockRequestArgs> requests)
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

    auto const new_expiry = clock::now() + req->ttl<millis>();

    if ((cached_block != cache->end() || cached_block != store->end())
        && cached_block.complete()) {
      prt.erase(req->block);
     //Add a pending request to facilitate content complete
      //Otherwise, the content complete event will not be poped up.
      prt.emplace(
          req->block,
          PendingRequest{clock::now(), new_expiry, req->keep_on_arrival});
      check_pending_requests(new_expiry);

//      already_have.push_back(std::move(req->block));

      // WARNING: don not use move copy
      already_have.push_back(req->block);
      req = requests.erase(req);
 
      continue;
    }


    auto it = prt.find(req->block);
    if (it != prt.end()) {
      // Avoid redudant request forwarding.
      requests.erase(req);
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
      ++req;
    }
  }

  std::uint16_t bytes_sent = 0;
  if (not requests.empty()) {
    log.d("Sending %v requests", requests.size());
    for (auto& req : requests) {
      log.d("| request block");
      log.d("| %v %v", req.block.hash, req.block.index);
    }
    bytes_sent = node.post(BlockRequest(std::move(requests)));
  }

  // WARNING: caller must be reentrant if the callback invokes it!
  // For example: the caller enters this function while looping over a
  // collection that it modifies. If the block arrived handler reenters that
  // caller, the inner execution will modify the collection while the outer
  // execution is still iterating, invalidating its iterators.
  for (auto& block : already_have) {
    log.d("block %v %v is locally stored/cached", block.hash, block.index);
    // check the pending request table to facilitate content complete.
    auto pending = prt.find(block);
    // There should be one pending request from the node itself.
    assert (pending != prt.end());

    // the block may exist only in the cache.
    // copy it from cache to store if keep_on_arrival = true
    // otherwise, store->validate will fail
    if (pending->second.keep_on_arrival
       and (store->find(block) == store->end())) {
      log.d("copying block %v %v from cache to store", block.hash, block.index); 
      ContentCache::copy_block_from_to(*cache, *store, block);
    }

    for (auto const& meta : rmt)
      if (meta.data.hash == block.hash) {
        if (store->validate_data(meta.data)) {
          content_complete_event(block.hash); 
        } 
      }
    block_arrived_event(node.id, block);
  }

  already_in_request = false;
  
  return bytes_sent;
}

void ContentHelperImpl::request_content (Hash content_name, 
                                         float utility_per_block, 
                                         std::uint32_t ttl)
{
  // check local meta
  for (auto const & local_meta : lmt) {
    if (local_meta.data.hash == content_name) {
      node.log.i ("Content %v is self published.", content_name);
      content_complete_event(content_name);
      return; // more complicated function can be added, e.g., return data
    }
  }

  // check remote meta
  int num_of_blocks = 0;
  for (auto const & remote_meta : rmt) {
    if (remote_meta.data.hash == content_name) {
      num_of_blocks = 1 + (remote_meta.data.size-1)/(remote_meta.data.block_size);
    }
  }

  assert (num_of_blocks != 0); // disable for real implementation
  if (num_of_blocks == 0)
    return;

  int* shuffle_idx_arr = new int [num_of_blocks];
  //shuffle
  for (std::size_t i=0; i<num_of_blocks; i++)
    shuffle_idx_arr[i] = i;

  std::random_shuffle (shuffle_idx_arr, shuffle_idx_arr + num_of_blocks);
  std::vector<BlockRequestArgs> requests;
  for (std::size_t i=0; i<num_of_blocks; i++) {
    BlockRef block (content_name, shuffle_idx_arr[i]);
    auto cached_block = cache->find(block);

    if (cached_block == cache->end())
      cached_block = store->find(block);

//    if (cached_block != cache->end() || cached_block != store->end()
//        && cached_block.complete()){
//      continue;
//    }

    requests.push_back (BlockRequestArgs(BlockRef(content_name, shuffle_idx_arr[i]),
                                         utility_per_block,
                                         std::chrono::milliseconds(ttl),
                                         node.id,
                                         node.position(),
                                         0,
                                         true));
  }

  delete[] shuffle_idx_arr;
  if (requests.size() > 0)
    this->request (requests); 
  else
    content_complete_event(content_name);
}

void ContentHelperImpl::receive(MessageHeader header, BlockRequest msg)
{
  log.d("Got %v requests", msg.requests.size());

  if (msg.requests.empty())
    return;

  blocks_requested_event(header.sender, std::move(msg.requests));

  if (not auto_respond and not auto_forward_requests)
    return;

/*  for (auto const& req : msg.requests) {
    if (auto_respond) {
      broadcast(req.block);

    } else if (auto_forward_requests) {
      auto prt_search = prt.find(req.block);
      if (prt_search != prt.end()) {
        auto& pending = prt_search->second;
        pending.local_only = false;
        auto new_expiry = clock::now() + req.ttl<millis>();
        if (pending.expiry < new_expiry) {
          pending.expiry = new_expiry;
          check_pending_requests(new_expiry);
        }
      }
    }
  }
  */
}


bool ContentHelperImpl::broadcast(BlockRef ref, std::uint16_t & bytes_sent)
{
  ContentCache* source = nullptr;
  auto block = cache->find(ref);
  if (block == cache->end()) {
    block = store->find(ref);
    if (block == store->end()) {
//      bytes_sent = 0;
      return false;
    }
  }

  log.d("Send block");
  log.d("| %v", ref);
  log.d("| size: %v bytes", block.size());
  log.d("");

  bytes_sent += node.post(BlockResponse(ref, block.data(), block.size()));

  return true;
}


void ContentHelperImpl::receive(MessageHeader header, BlockResponse msg)
{
  log.d("Got block from node %v", header.sender);
  log.d("| %v", msg.block);

  bool is_stored = false;
  bool will_rebroadcast = false;

  // Skip the cache if we're expecting to keep all this content's data.
  auto pending = prt.find(msg.block);
  if (pending != prt.end()) {
    if (pending->second.keep_on_arrival
        and (store->find(msg.block) == store->end())) {
      log.d("| I'll store this permanently");
      store->store(msg.block, msg.size, msg.data, msg.size);
      for (auto const& meta : rmt)
        if (meta.data.hash == msg.block.hash) {
          if (store->validate_data(meta.data)) {
//            log.i("Content %v complete at %v",
//                  msg.block.hash,
//                  std::chrono::duration_cast<millis>(
//                      clock::now().time_since_epoch()).count());
            content_complete_event(msg.block.hash);
          }
          break;
        }
      is_stored = true;
    }

//    will_rebroadcast = auto_respond and not pending->second.local_only;
  }

  // Cache all blocks we come across
  if (not is_stored and (cache->find(msg.block) == cache->end())) {
    cache->store(msg.block, msg.size, msg.data, msg.size);
    log.i ("cached block %v %v opportunistically", msg.block.hash, msg.block.index);
  }

  block_arrived_event(header.sender, msg.block);

//  if (will_rebroadcast) {
//    log.d("| I'll rebroadcast it, too");
//    broadcast(msg.block);
//  }
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

Event<NodeId, BlockRef>& ContentHelperImpl::on_block_arrived()
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
  // 2^t growth
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
