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
  : inbox_(inbox)
{
  assert(inbox);
}
channel::channel(channel&& r)
  : inbox_(r.inbox_)
{
  r.inbox_ = nullptr;
}
channel::channel(const channel& r)
  : inbox_(r.inbox_)
{
}
channel& channel::operator=(channel&& r)
{
  std::swap(inbox_, r.inbox_);
  return *this;
}
channel& channel::operator=(const channel& r)
{
  inbox_ = r.inbox_;
  return *this;
}
channel::~channel() {}
}
