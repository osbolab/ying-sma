#include <sma/channel.hpp>
#include <sma/sink.hpp>

#include <cassert>
#include <utility>


namespace sma
{
channel::channel()
{
}
channel::channel(csink<message>* inbound)
  : inbound(inbound)
{
  assert(inbound);
}
channel::channel(channel&& rhs)
  : inbound(rhs.inbound)
{
  rhs.inbound = nullptr;
}
channel::channel(const channel& rhs)
  : inbound(rhs.inbound)
{
}
channel& channel::operator=(channel&& rhs)
{
  std::swap(inbound, rhs.inbound);
  return *this;
}
channel& channel::operator=(const channel& rhs)
{
  inbound = rhs.inbound;
  return *this;
}
channel::~channel() {}
}
