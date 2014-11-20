#include <sma/channel.hpp>
#include <sma/sink.hpp>

#include <cassert>
#include <utility>


namespace sma
{
channel::channel()
{
}
channel::channel(sink<message const&>* inbox)
  : inbox(inbox)
{
  assert(inbox);
}
channel::channel(channel&& r)
  : inbox(r.inbox)
{
  r.inbox = nullptr;
}
channel::channel(const channel& r)
  : inbox(r.inbox)
{
}
channel& channel::operator=(channel&& r)
{
  std::swap(inbox, r.inbox);
  return *this;
}
channel& channel::operator=(const channel& r)
{
  inbox = r.inbox;
  return *this;
}
channel::~channel() {}
}
