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
NeighborHelperImpl::NeighborHelperImpl(CcnNode& node)
  : NeighborHelper(node)
{
  schedule_beacon(100ms);
}

void NeighborHelperImpl::saw(NodeId const& node) { neighbors.update(node); }

void NeighborHelperImpl::saw(std::vector<NodeId> const& nodes)
{
  for (auto const& node : nodes)
    saw(node);
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
  asynctask(std::bind(&NeighborHelperImpl::beacon, this)).do_in(delay);
}


void NeighborHelperImpl::beacon()
{
  node->post(Beacon());

  schedule_beacon(3s);
}

void NeighborHelperImpl::prune_neighbors()
{
  if (std::uint32_t(node->id) != 0)
    return;

  if (!neighbors.empty()) {
    neighbors.prune(prune_interval);
    if (!neighbors.empty()) {
      log.d("neighbors (%v):", neighbors.size());
      std::ostringstream s;
      for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
        s << " " << std::setfill(' ') << std::setw(3) << it->first << " "
          << sma::chrono::system_clock::utcstrftime(it->second.last_seen);
        log.d(s.str());
        s.str("");
        s.clear();
      }

      log.d("");
    }
  }
  asynctask(std::bind(&NeighborHelperImpl::prune_neighbors, this))
      .do_in(prune_interval);
}
}
