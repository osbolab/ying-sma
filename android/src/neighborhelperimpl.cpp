#include <sma/neighborhelperimpl.hpp>

#include <sma/nodeid.hpp>
#include <sma/ccn/ccnnode.hpp>

#include <sma/messageheader.hpp>
#include <sma/beacon.hpp>

#include <sma/async.hpp>
#include <sma/context.hpp>

#include <sstream>
#include <iomanip>

#include <chrono>

namespace sma
{

constexpr unsigned int NeighborHelperImpl::INITIAL_REFRESH_MS;

NeighborHelperImpl::NeighborHelperImpl(CcnNode& node)
  : NeighborHelper(node)
{
  schedule_beacon(std::chrono::milliseconds(100));
  schedule_refresh();
}

std::vector<Neighbor> NeighborHelperImpl::get() const
{
  std::vector<Neighbor> v;
  for (auto& pair : neighbors)
    v.push_back(
        Neighbor{pair.first, pair.second.position(), pair.second.velocity});
  return v;
}

void NeighborHelperImpl::saw(NodeId const& node, Vec2d const& position)
{
  auto it = neighbors.find(node);
  if (it == neighbors.end()) {
    neighbors.emplace(node, NeighborRecord(position));
  } else {
    it->second.saw(position);
  }
}

void NeighborHelperImpl::receive(MessageHeader header, Beacon msg)
{
  auto const pre_count = neighbors.size();
  saw(header.sender, msg.position);
  std::ostringstream ss;
  auto count = neighbors.size();
  if (count != pre_count) {
    for (auto& pair : neighbors) {
      ss << pair.first;
      if (--count != 0)
        ss << ", ";
    }
    log.t("neighbors: %v", ss.str());
  }

  if (!msg.is_response)
    node.post(Beacon(node.position(), true));
}

using millis = std::chrono::milliseconds;
void NeighborHelperImpl::schedule_beacon(millis delay)
{
  using unit = millis::rep;
  unit min = delay.count() / 2;
  delay = millis(min + rand() % delay.count());
  asynctask(&NeighborHelperImpl::beacon, this).do_in(delay);
}

void NeighborHelperImpl::beacon()
{
  node.post(Beacon(node.position()));
  schedule_beacon(std::chrono::seconds(2));
}

void NeighborHelperImpl::refresh_neighbors()
{
  if (neighbors.empty()) {
    schedule_refresh();
    return;
  }

  log.t("refreshing %v neighbors", neighbors.size());

  bool will_beacon = false;
  std::vector<NodeId> dropped;

  auto it = neighbors.begin();
  while (it != neighbors.end()) {
    if (it->second.older_than(millis(INITIAL_REFRESH_MS))) {
      if (it->second.times_pinged++ < 3) {
        will_beacon = true;
      } else {
        dropped.push_back(it->first);
        it = neighbors.erase(it);
        continue;
      }
    }
    ++it;
  }
  auto refresh_delay = std::chrono::milliseconds(INITIAL_REFRESH_MS);

  if (will_beacon) {
    beacon();
    refresh_delay = query_response_delay;
  }

  schedule_refresh(refresh_delay);

  if (not dropped.empty()) {
    log.t("dropping %v neighbors", dropped.size());
    on_departure(std::move(dropped));
  }
}

void NeighborHelperImpl::schedule_refresh(millis const& delay)
{
  asynctask(&NeighborHelperImpl::refresh_neighbors, this).do_in(delay);
}
}
