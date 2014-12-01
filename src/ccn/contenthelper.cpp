#include <sma/ccn/contenthelper.hpp>

#include <sma/context.hpp>
#include <sma/ccn/ccnnode.hpp>

#include <sma/messageheader.hpp>
#include <sma/ccn/contentinfomessage.hpp>

namespace sma
{
ContentHelper::ContentHelper(CcnNode& node)
  : node(&node)
  , log(node.context->log)
{
}

void ContentHelper::receive(MessageHeader header, ContentInfoMessage msg)
{
  log.d("Content info from %v", header.sender);
}
}
