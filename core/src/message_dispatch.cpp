#include <sma/message_dispatch.hpp>
#include <sma/message.hpp>
#include <sma/sink.hpp>
#include <sma/rws_mutex.hpp>
#include <sma/log.hpp>

#include <cstdint>
#include <cassert>
#include <memory>


namespace sma
{

std::unique_ptr<message_dispatch>
message_dispatch::new_single_threaded(node::id this_sender)
{
  return std::unique_ptr<message_dispatch>(
      new message_dispatch(std::move(this_sender)));
}

std::unique_ptr<message_dispatch>
message_dispatch::new_concurrent(node::id this_sender)
{
  return std::unique_ptr<message_dispatch>(
      new detail::concurrent_dispatch(std::move(this_sender)));
}

message_dispatch::message_dispatch(node::id this_sender,
                                   csink<message>* outbound)
  : this_sender(std::move(this_sender))
  , outbound(outbound)
{
  if (!outbound)
    LOG(WARNING) << "Messenger has no outbound channel.";
}
message_dispatch::message_dispatch(node::id this_sender)
  : message_dispatch(this_sender, nullptr)
{
}

message_dispatch::message_dispatch(message_dispatch&& rhs)
  : handlers(std::move(rhs.handlers))
  , next_id(rhs.next_id.load())
  , this_sender(std::move(rhs.this_sender))
  , outbound(rhs.outbound)
{
  rhs.outbound = nullptr;
}
message_dispatch& message_dispatch::operator=(message_dispatch&& rhs)
{
  std::swap(handlers, rhs.handlers);
  next_id.store(rhs.next_id.load());
  std::swap(this_sender, rhs.this_sender);
  std::swap(outbound, rhs.outbound);
  return *this;
}

message_dispatch& message_dispatch::post_via(csink<message>* outbound)
{
  this->outbound = outbound;
  if (!outbound)
    LOG(WARNING) << "Messenger has no outbound channel.";
  return *this;
}

messenger& message_dispatch::post(const message::stub& stub)
{
  if (outbound)
    outbound->accept(stub.build(this_sender, next_id.fetch_add(1)));
  else
    LOG(WARNING) << "Message dropped: messenger has no outbound channel";
  return *this;
}

messenger& message_dispatch::subscribe(message::domain_type domain,
                                       messenger::handler h)
{
  // We could use a binary search to find the insertion point, but
  // it still then requires an O(n) step to create the gap, so
  // we'd have an O(log) search and an O(n) insert.
  // Instead just do them at the same time by sliding in the back (heh)

  handlers.emplace_back(std::move(domain), std::move(h));
  auto i = handlers.size();
  while (--i > 0 && handlers[i].first < handlers[i - 1].first)
    std::swap(handlers[i - 1], handlers[i]);
  return *this;
}

void message_dispatch::accept(const message& msg)
{
  bool handled = false;
  if (!handlers.empty()) {
    for (std::size_t i = 0; i < handlers.size(); ++i) {
      if (handlers[i].first == msg.domain()) {
        while (handlers[i].first == msg.domain())
          handlers[i++].second(msg);
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
  concurrent_dispatch::concurrent_dispatch(node::id this_sender,
                                           csink<message>* outbound)
    : message_dispatch(std::move(this_sender), std::move(outbound))
  {
  }
  concurrent_dispatch::concurrent_dispatch(node::id this_sender)
    : message_dispatch(std::move(this_sender))
  {
  }

  messenger& concurrent_dispatch::subscribe(message::domain_type domain,
                                            messenger::handler h)
  {
    {
      writer_lock lock(mx);
      message_dispatch::subscribe(std::move(domain), std::move(h));
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
