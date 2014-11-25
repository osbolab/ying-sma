#include <sma/ccn/ccnnode.hpp>

#include <sma/message.hpp>
#include <sma/ccn/ccnmessage.hpp>

namespace sma
{
CcnNode::CcnNode(NodeId id, Context* ctx)
  : Node(id, ctx)
{
}

void CcnNode::dispose() {}

void CcnNode::receive(Message const& msg)
{
  switch (msg.type()) {
    default:
      Node::receive(msg);
  }
}
}
