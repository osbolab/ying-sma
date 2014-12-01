#include <sma/ccn/ccnnode.hpp>

#include <sma/context.hpp>
#include <sma/messageheader.hpp>

#include <sma/neighbormessage.hpp>
#include <sma/ccn/interestmessage.hpp>
#include <sma/ccn/contentinfomessage.hpp>

namespace sma
{
CcnNode::CcnNode(NodeId id, Context& context)
  : id(id)
  , context(&context)
  , linklayer(context.linklayer)
  , log(context.log)
  , neighbor_helper(*this)
  , interest_helper(*this)
  , content_helper(*this)
{
}

void CcnNode::receive(MessageHeader&& header, NeighborMessage&& msg)
{
  neighbor_helper.receive(std::move(header), std::move(msg));
}

void CcnNode::receive(MessageHeader&& header, InterestMessage&& msg)
{
  interest_helper.receive(std::move(header), std::move(msg));
}

void CcnNode::receive(MessageHeader&& header, ContentInfoMessage&& msg)
{
  content_helper.receive(std::move(header), std::move(msg));
}
}
