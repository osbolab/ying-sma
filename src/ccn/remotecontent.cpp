#include <sma/ccn/remotecontent.hpp>

#include <sma/ccn/ccnnode.hpp>
#include <sma/ccn/contenthelper.hpp>
#include <sma/ccn/contentstore.hpp>


namespace sma
{
RemoteContent::RemoteContent(CcnNode& node,
                             ContentHelper& helper,
                             ContentStore& store,
                             ContentMetadata metadata,
                             NetworkDistance distance)
  : node(node)
  , helper(helper)
  , store(store)
  , meta(metadata)
  , dist(distance)
{
}

void RemoteContent::fetch_block(std::uint32_t index,
                                std::chrono::milliseconds timeout)
{
  assert(index < (1 + (meta.size - 1) / meta.block_size));

  std::vector<BlockFragmentRequest> fragments;

  auto cached = store.find(meta.hash);
  if (cached != nullptr) {
  }

  // node->post(BlockRequest(metadata_.hash, index));
}

bool RemoteContent::saw(NetworkDistance distance)
{
  touch();
  if (distance >= dist)
    return false;

  dist = distance;
  return true;
}
}
