#include <sma/ccn/ccnnode.hpp>

#include <sma/context.hpp>
#include <sma/messageheader.hpp>

#include <sma/neighborhelper.hpp>
#include <sma/neighbormessage.hpp>

#include <sma/ccn/interesthelper.hpp>
#include <sma/ccn/interestmessage.hpp>

#include <sma/ccn/contenthelper.hpp>
#include <sma/ccn/contentinfomessage.hpp>

namespace sma
{
CcnNode::CcnNode(NodeId id, Context& context)
  : id(id)
  , context(&context)
  , linklayer(context.linklayer)
  , log(context.log)
  , neighbor_helper(new NeighborHelper(*this))
  , interest_helper(new InterestHelper(*this))
  , content_helper(new ContentHelper(*this))
{
}

CcnNode::~CcnNode()
{
  delete content_helper;
  delete interest_helper;
  delete neighbor_helper;
}
Neighbors& CcnNode::neighbors()
{
  return static_cast<Neighbors&>(*neighbor_helper);
}

Interests& CcnNode::interests()
{
  return static_cast<Interests&>(*interest_helper);
}

Content& CcnNode::content() { return static_cast<Content&>(*content_helper); }

void CcnNode::receive(MessageHeader&& header, NeighborMessage&& msg)
{
  if (neighbor_helper)
    neighbor_helper->receive(std::move(header), std::move(msg));
}

void CcnNode::receive(MessageHeader&& header, InterestMessage&& msg)
{
  if (interest_helper)
    interest_helper->receive(std::move(header), std::move(msg));
}

void CcnNode::receive(MessageHeader&& header, ContentInfoMessage&& msg)
{
  if (content_helper)
    content_helper->receive(std::move(header), std::move(msg));
}
}
