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
  : ibx(inbox)
{
  assert(inbox);
}
channel::channel(channel&& r)
  : ibx(r.ibx)
{
  r.ibx = nullptr;
}
channel::channel(const channel& r)
  : ibx(r.ibx)
{
}
channel& channel::operator=(channel&& r)
{
  std::swap(ibx, r.ibx);
  return *this;
}
channel& channel::operator=(const channel& r)
{
  ibx = r.ibx;
  return *this;
}
channel::~channel() {}
}
