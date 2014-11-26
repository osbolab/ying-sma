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

void CcnNode::receive(Message msg)
{
  switch (msg.type) {
    case InterestMessage::TYPE: {
      receive_into<InterestMessage>(std::move(msg), interestHelper);
      break;
    }

    default:
      Node::receive(std::move(msg));
  }
}

void CcnNode::add_interests(std::vector<ContentType> types)
{
  interestHelper.insert_new(std::move(types));
}
}
