#include <sma/message_dispatch.hpp>
#include <sma/message.hpp>
#include <sma/actor.hpp>
#include <sma/sink.hpp>
#include <sma/rws_mutex.hpp>
#include <sma/log>

#include <cstdint>
#include <cassert>
#include <memory>


namespace sma
{

std::unique_ptr<message_dispatch> message_dispatch::new_single_threaded()
{
  return std::unique_ptr<message_dispatch>(new message_dispatch());
}

std::unique_ptr<message_dispatch> message_dispatch::new_concurrent()
{
  return std::unique_ptr<message_dispatch>(new detail::concurrent_dispatch());
}

message_dispatch::message_dispatch(sink<message const&>* outbound)
  : outbound(outbound)
{
}
message_dispatch::message_dispatch()
  : message_dispatch(nullptr)
{
}

message_dispatch::message_dispatch(message_dispatch&& rhs)
  : subs(std::move(rhs.subs))
  , outbound(rhs.outbound)
{
  rhs.outbound = nullptr;
}
message_dispatch& message_dispatch::operator=(message_dispatch&& rhs)
{
  std::swap(subs, rhs.subs);
  std::swap(outbound, rhs.outbound);
  return *this;
}

message_dispatch& message_dispatch::post_via(sink<message const&>* outbound)
{
  this->outbound = outbound;
  if (!outbound)
    LOG(WARNING) << "Messenger has no outbound channel.";
  return *this;
}

messenger& message_dispatch::post(message const& msg)
{
  if (outbound)
    outbound->accept(msg);
  else
    LOG(WARNING) << "Message dropped: messenger has no outbound channel";
  return *this;
}

messenger& message_dispatch::subscribe(message_type type, actor* subscriber)
{
  // We could use a binary search to find the insertion point, but
  // it still then requires an O(n) step to create the gap, so
  // we'd have an O(log) search and an O(n) insert.
  // Instead just do them at the same time by sliding in the back (heh)

  subs.emplace_back(std::move(type), std::move(subscriber));
  auto i = subs.size();
  while (--i > 0 && subs[i].first < subs[i - 1].first)
    std::swap(subs[i - 1], subs[i]);
  return *this;
}

void message_dispatch::accept(const message& msg)
{
  bool handled = false;
  if (!subs.empty()) {
    for (std::size_t i = 0; i < subs.size(); ++i) {
      if (subs[i].first == msg.type()) {
        while (subs[i].first == msg.type())
          subs[i++].second->on_message(msg);
        handled = true;
      } else if (handled)
        return;
    }
  }
  if (!handled)
    LOG(WARNING) << "Unhandled message:\n" << msg;
}

namespace detail
{
  concurrent_dispatch::concurrent_dispatch(sink<message const&>* outbound)
    : message_dispatch(std::move(outbound))
  {
  }
  concurrent_dispatch::concurrent_dispatch()
    : message_dispatch()
  {
  }

  messenger& concurrent_dispatch::subscribe(message_type type, actor* subscriber)
  {
    {
      writer_lock lock(mx);
      message_dispatch::subscribe(std::move(type), std::move(subscriber));
    }
    return *this;
  }

  void concurrent_dispatch::accept(const message& msg)
  {
    {
      reader_lock lock(mx);
      message_dispatch::accept(msg);
    }
  }
}
}
