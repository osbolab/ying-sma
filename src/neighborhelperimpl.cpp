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

using namespace std::literals::chrono_literals;

namespace sma
{

constexpr unsigned int NeighborHelperImpl::INITIAL_REFRESH_MS;

NeighborHelperImpl::NeighborHelperImpl(CcnNode& node)
  : NeighborHelper(node)
{
  schedule_beacon(100ms);
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
  if (it == neighbors.end())
    neighbors.emplace(node, NeighborRecord(position));
  else {
    it->second.saw(position);
    log.d("Neighbor %v", it->first);
    log.d("| position: %v", std::string(it->second.position()));
    log.d("| velocity: %v m/s", it->second.velocity.length());
  }
}

void NeighborHelperImpl::receive(MessageHeader header, Beacon msg)
{
  saw(header.sender, msg.position);
  for (auto& n : get()) {
    log.d("Neighbor %v", n.id);
    log.d("| position: %v", std::string(n.position));
    log.d("| velocity: %v", std::string(n.velocity));
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

void NeighborHelperImpl::beacon() { node.post(Beacon(node.position())); }

void NeighborHelperImpl::refresh_neighbors()
{
  if (neighbors.empty())
    return;

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
  if (will_beacon) {
    beacon();
    schedule_refresh(query_response_delay);
  }

  on_departure(std::move(dropped));
}

void NeighborHelperImpl::schedule_refresh(millis const& delay)
{
  asynctask(&NeighborHelperImpl::refresh_neighbors, this).do_in(delay);
}
}
