#include <sma/node.hpp>
#include <sma/message.hpp>
#include <sma/neighbormessage.hpp>

#include <sma/binaryformatter.hpp>
#include <sma/bytearrayreader.hpp>

#include <vector>

#include <sstream>
#include <iomanip>

namespace sma
{
constexpr decltype(Node::INITIAL_BEACON_DELAY) Node::INITIAL_BEACON_DELAY;

using millis = std::chrono::milliseconds;

Node::Node(NodeId id, Context* ctx)
  : Actor(ctx)
  , id_(id)
  // prime for better randomization of lower IDs
  , rand(std::default_random_engine(std::uint64_t(id) * 2305843009213693951L))
{
  subscribe(NeighborMessage::TYPE);
  schedule_beacon(INITIAL_BEACON_DELAY);
  prune_neighbors();
}

Node::~Node() { }

void Node::receive(Message const& msg)
{
  if (msg.type() == NeighborMessage::TYPE) {
    log.t("<-- beacon (%v bytes)", msg.size());
    auto nm = NeighborMessage::read(msg.cdata(), msg.size());
    neighbors.update(nm.sender);
  }
}

void Node::schedule_beacon(millis delay)
{
  using unit = millis::rep;
  unit min = delay.count() / 2;
  delay = millis(min + rand() % delay.count());
  async(std::bind(&Node::beacon, this)).do_in(delay);
}

void Node::beacon()
{
  auto msg = NeighborMessage(id(), beacon_data()).to_message();
  log.t("--> beacon (%v bytes)", msg.size());
  post(msg);

  schedule_beacon(beacon_delay);
}

NeighborMessage::body_type Node::beacon_data()
{
  return NeighborMessage::body_type();
}

void Node::prune_neighbors()
{
  if (std::uint32_t(id()) != 0)
    return;

  if (!neighbors.empty()) {
    std::vector<NeighborTable::value_type> pruned;
    neighbors.prune(prune_interval, &pruned);
    if (!pruned.empty()) {
      std::ostringstream s;
      for (auto& n : pruned)
        s << n.first << ", ";
      //log.d("dropped neighbors: [%v]", s.str());
    }
    if (!neighbors.empty()) {
      log.d("neighbors (%v):", neighbors.size());
      std::ostringstream s;
      for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
        s << " " << std::setfill(' ') << std::setw(3) << it->first << " "
          <<  sma::chrono::system_clock::utcstrftime(it->second.last_seen);
        log.d(s.str());
        s.str("");
        s.clear();
      }

      log.d("");
    }
  }
  async(std::bind(&Node::prune_neighbors, this)).do_in(prune_interval);
}
}
