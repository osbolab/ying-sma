#include <sma/ccn/ccnnode.hpp>

#include <sma/context.hpp>
#include <sma/linklayer.hpp>
#include <sma/gpscomponent.hpp>

#include <sma/messageheader.hpp>

#include <sma/neighborhelper.hpp>
#include <sma/ccn/interesthelper.hpp>
#include <sma/ccn/contenthelper.hpp>

#include <sma/stats.hpp>

#include <cassert>

namespace sma
{
CcnNode::CcnNode(NodeId id, Context& context)
  : id(id)
  , context(&context)
  , linklayer(context.linklayer)
  , log(context.log)
{
  stats::all_nodes.push_back(id);
}

Vec2d CcnNode::position() const
{
  auto const* gps = context->template try_get_component<GpsComponent>();
  assert(gps);
  auto pos = gps->position();
  return Vec2d(pos.lon, pos.lat);
}

void CcnNode::post(void const* src, std::size_t size)
{
  assert(linklayer);
  linklayer->enqueue(src, size);
}

void CcnNode::receive(MessageHeader header, Beacon msg)
{
  assert(neighbors);
  neighbors->receive(std::move(header), std::move(msg));
}

void CcnNode::receive(MessageHeader header, InterestAnn msg)
{
  assert(interests);
  interests->receive(std::move(header), std::move(msg));
}

void CcnNode::receive(MessageHeader header, ContentAnn msg)
{
  assert(content);
  content->receive(std::move(header), std::move(msg));
}

void CcnNode::receive(MessageHeader header, BlockRequest msg)
{
  assert(content);
  content->receive(std::move(header), std::move(msg));
}

void CcnNode::receive(MessageHeader header, BlockResponse msg)
{
  assert(content);
  content->receive(std::move(header), std::move(msg));
}
}
