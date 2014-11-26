#include <sma/node.hpp>
#include <sma/message.hpp>
#include <sma/neighbormessage.hpp>

#include <vector>

namespace sma
{
Node::Node(NodeId id, Context* ctx)
  : Actor(ctx)
  , id_(id)
{
  subscribe(NeighborMessage::TYPE);
}

Node::~Node() {}

void Node::receive(Message&& msg)
{
  switch (msg.type) {
    case NeighborMessage::TYPE:
      auto nm = NeighborMessage(msg.body.cdata(), msg.body.size());
      neighborHelper.receive(std::move(msg), std::move(nm));
      break;

    default:
      log.w("unhandled message: %v", msg.type);
  }
}
}
