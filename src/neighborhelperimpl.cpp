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

void NeighborHelperImpl::saw(NodeId const& node)
{
  auto result = neighbors.emplace(std::move(node), Neighbor());
  if (!result.second)
    result.first->second.saw();
}

void NeighborHelperImpl::receive(MessageHeader header, Beacon msg)
{
  saw(header.sender);
  if (!msg.is_response)
    node->post(Beacon(true));
}

using millis = std::chrono::milliseconds;
void NeighborHelperImpl::schedule_beacon(millis delay)
{
  using unit = millis::rep;
  unit min = delay.count() / 2;
  delay = millis(min + rand() % delay.count());
  asynctask(&NeighborHelperImpl::beacon, this).do_in(delay);
}

void NeighborHelperImpl::beacon() { node->post(Beacon()); }

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
