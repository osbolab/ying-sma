#include <sma/node.hpp>
#include <sma/message.hpp>
#include <sma/neighbormessage.hpp>

#include <sma/bytearrayreader.hpp>
#include <sma/binaryformatter.hpp>

using namespace std::literals::chrono_literals;

namespace sma
{
constexpr decltype(Node::INITIAL_BEACON_DELAY) Node::INITIAL_BEACON_DELAY;
constexpr decltype(Node::BEACON_DELAY) Node::BEACON_DELAY;

Node::Node(NodeId id, Context* ctx)
  : Actor(ctx)
  , id_(id)
{
  schedule_beacon(INITIAL_BEACON_DELAY);
}

Node::~Node() { dispose(); }

void Node::dispose() { disposed = true; }

void Node::receive(Message const& msg)
{
  if (msg.type() == NeighborMessage::TYPE) {
    auto reader
        = ByteArrayReader(msg.cdata(), msg.size()).format<BinaryFormatter>();
    auto nm = reader.template get<NeighborMessage>();
    if (neighbors.update(nm.sender).is_new())
      log.i("discovered neighbor: %v", nm.sender);
  }
}

void Node::schedule_beacon(std::chrono::milliseconds delay)
{
  if (!disposed)
    async(std::bind(&Node::beacon, this)).do_in(delay);
}

void Node::beacon()
{
  if (disposed)
    return;

  schedule_beacon(BEACON_DELAY);
}
}
