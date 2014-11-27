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
  , interestHelper(this)
{
}

void Node::receive(MessageHeader header, NeighborMessage msg)
{
}

void Node::receive(MessageHeader header, InterestMessage msg)
{
  interest_helper.receive(std::move(header), std::move(msg));
}

void Node::add_interests(std::vector<ContentType> types)
{
  interestHelper.insert_new(std::move(types));
}
}
