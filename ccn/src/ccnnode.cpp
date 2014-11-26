#include <sma/ccn/ccnnode.hpp>

#include <sma/message.hpp>
#include <sma/ccn/interestmessage.hpp>

namespace sma
{
CcnNode::CcnNode(NodeId id, Context* ctx)
  : Node(id, ctx)
  , interestHelper(this)
{
  subscribe(InterestMessage::TYPE);
}

void CcnNode::receive(Message&& msg)
{
  switch (msg.type()) {
    case InterestMessage::TYPE:
      interestHelper.receive(InterestMessage::read(msg.cdata(), msg.size()));
      break;
    default:
      Node::receive(std::move(msg));
  }
}

void CcnNode::make_interest(std::vector<ContentType> types)
{
}
}
