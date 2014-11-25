#include <sma/ccn/ccnnode.hpp>

#include <sma/message.hpp>
#include <sma/ccn/interestmessage.hpp>

namespace sma
{
CcnNode::CcnNode(NodeId id, Context* ctx)
  : Node(id, ctx)
  , interestHelper(log)
{
  subscribe(InterestMessage::TYPE);
}

void CcnNode::receive(Message const& msg)
{
  switch (msg.type()) {
    case InterestMessage::TYPE:
      interestHelper.receive(InterestMessage::read(msg.cdata(), msg.size()));
      break;
    default:
      Node::receive(msg);
  }
}
}
