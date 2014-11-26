#include <sma/node.hpp>
#include <sma/message.hpp>
#include <sma/neighbormessage.hpp>

#include <vector>

namespace sma
{
Node::Node(NodeId id, Context* ctx)
  : Actor(ctx)
  , id_(id)
  , neighborHelper(this)
{
  subscribe(NeighborMessage::TYPE);
}

void Node::receive(Message msg)
{
  switch (msg.type) {
    case NeighborMessage::TYPE: {
      receive_into<NeighborMessage>(std::move(msg), neighborHelper);
      break;
    }

    default:
      log.w("unhandled message: %v", msg.type);
  }
}
}
