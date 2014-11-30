#include <sma/neighborhelper.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/messageheader.hpp>
#include <sma/neighbormessage.hpp>

#include <sma/async.hpp>
#include <sma/context.hpp>

#include <sstream>
#include <iomanip>

#include <chrono>

using namespace std::literals::chrono_literals;

namespace sma
{
NeighborHelper::NeighborHelper(CcnNode* node)
  : node(node)
  , log(node->context->log)
{
  schedule_beacon(100ms);
}

void NeighborHelper::receive(MessageHeader&& header, NeighborMessage&& msg)
{
  neighbors.update(header.sender);
}

using millis = std::chrono::milliseconds;
void NeighborHelper::schedule_beacon(millis delay)
{
  using unit = millis::rep;
  unit min = delay.count() / 2;
  delay = millis(min + rand() % delay.count());
  asynctask(std::bind(&NeighborHelper::beacon, this)).do_in(delay);
}


void NeighborHelper::beacon()
{
  node->post(NeighborMessage());

  schedule_beacon(std::chrono::milliseconds(3000));
}

void NeighborHelper::prune_neighbors()
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
  asynctask(std::bind(&NeighborHelper::prune_neighbors, this))
      .do_in(prune_interval);
}
}
