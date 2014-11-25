#include <sma/node.hpp>
#include <sma/message.hpp>
#include <sma/neighbormessage.hpp>

#include <sma/binaryformatter.hpp>
#include <sma/bytearrayreader.hpp>

#include <iostream>
#include <iomanip>

namespace sma
{
constexpr decltype(Node::INITIAL_BEACON_DELAY) Node::INITIAL_BEACON_DELAY;
constexpr decltype(Node::BEACON_DELAY) Node::BEACON_DELAY;

using millis = std::chrono::milliseconds;


Node::Node(NodeId id, Context* ctx)
  : Actor(ctx)
  , id_(id)
  , rand(std::default_random_engine(std::uint64_t(id)*2305843009213693951L))
{
  subscribe(NeighborMessage::TYPE);
  schedule_beacon(INITIAL_BEACON_DELAY);
}

Node::~Node() { dispose(); }

void Node::dispose() { disposed = true; }

void Node::receive(Message const& msg)
{
  if (msg.type() == NeighborMessage::TYPE) {
    log.t("<-- beacon (%v bytes)", msg.size());
  std::cout << "< ";
    for (int i = 0; i < msg.size(); ++i)
      std::cout << std::setw(2) << std::setfill('0') << std::hex << int(msg.cdata()[i]) << ' ';
    std::cout << std::endl;
    auto nm = NeighborMessage::read(msg.cdata(), msg.size());
    if (neighbors.update(nm.sender).is_new())
      log.i("discovered neighbor: %v", nm.sender);
    else
      log.d("updated neighbor: %v", nm.sender);
  }
}

void Node::schedule_beacon(millis delay)
{
  if (disposed)
    return;

  using unit = millis::rep;
  unit min = delay.count() / 2;
  delay = millis(min + rand() % delay.count());
  async(std::bind(&Node::beacon, this)).do_in(delay);
}

void Node::beacon()
{
  if (disposed)
    return;

  auto msg = NeighborMessage(id(), beacon_data()).to_message();
  std::cout << "id: " << id() << std::endl;
  std::cout << "> ";
    for (int i = 0; i < msg.size(); ++i)
      std::cout << std::setw(2) << std::setfill('0') << std::hex << int(msg.cdata()[i]) << ' ';
    std::cout << std::endl;
  log.t("--> beacon (%v bytes)", msg.size());
  post(msg);

  schedule_beacon(BEACON_DELAY);
}

NeighborMessage::body_type Node::beacon_data()
{
  return NeighborMessage::body_type();
}
}
