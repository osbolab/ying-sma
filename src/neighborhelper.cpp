#include <sma/neighborhelper.hpp>

#include <sma/node.hpp>
#include <sma/async.hpp>
#include <sma/context.hpp>
#include <sma/message.hpp>

#include <sstream>
#include <iomanip>

namespace sma
{
NeighborHelper::NeighborHelper(Node* node)
  : node(node)
  , log(node->context()->log())
{
}

void NeighborHelper::receive(Message&& msg, NeighborMessage&& nm)
{
  log.t("<-- beacon (%v bytes)", nm.body.size());
  neighbors.update(msg.sender);
}

using millis = std::chrono::milliseconds;
void NeighborHelper::schedule_beacon(millis delay)
{
  using unit = millis::rep;
  unit min = delay.count() / 2;
  delay = millis(min + rand() % delay.count());
  node->async(std::bind(&NeighborHelper::beacon, this)).do_in(delay);
}


void NeighborHelper::beacon()
{
  auto body = have_beacon_data ? std::move(next_beacon_data)
                               : NeighborMessage::body_type();

  auto msg = make_message<NeighborMessage>(node->id(), std::move(body));

  log.t("--> beacon (%v bytes)", msg.body.size());
  node->post(std::move(msg));

  schedule_beacon(std::chrono::milliseconds(3000));
}

void NeighborHelper::prune_neighbors()
{
  if (std::uint32_t(node->id()) != 0)
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
  node->async(std::bind(&NeighborHelper::prune_neighbors, this))
      .do_in(prune_interval);
}
}
