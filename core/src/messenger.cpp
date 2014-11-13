#include <sma/messenger.hpp>
#include <sma/message.hpp>
#include <sma/sink.hpp>
#include <sma/rws_mutex.hpp>
#include <sma/log.hpp>

#include <cstdint>


namespace sma
{

messenger messenger::new_single_threaded(node::id this_sender,
                                         sink<message>* outbound)
{
  return messenger(std::move(this_sender), outbound);
}

messenger messenger::new_concurrent(node::id this_sender,
                                    sink<message>* outbound)
{
  return detail::concurrent_messenger(std::move(this_sender), outbound);
}

messenger::messenger(messenger&& rhs)
  : handlers(std::move(rhs.handlers))
  , next_id(rhs.next_id.load())
  , this_sender(std::move(rhs.this_sender))
  , outbound(rhs.outbound)
{
  rhs.outbound = nullptr;
}
messenger& messenger::operator=(messenger&& rhs)
{
  std::swap(handlers, rhs.handlers);
  next_id.store(rhs.next_id.load());
  std::swap(this_sender, rhs.this_sender);
  std::swap(outbound, rhs.outbound);
  return *this;
}

void messenger::send(const message::builder& builder)
{
  outbound->accept(builder.build(this_sender, next_id.fetch_add(1)));
}

void messenger::subscribe(message::domain_type domain, msg_handler h)
{
  // We could use a binary search to find the insertion point, but
  // it still then requires an O(n) step to create the gap, so
  // we'd have an O(log) search and an O(n) insert.
  // Instead just do them at the same time by sliding in the back (heh)

  handlers.emplace_back(std::move(domain), std::move(h));
  auto i = handlers.size();
  while (--i > 0 && handlers[i].first < handlers[i - 1].first)
    std::swap(handlers[i - 1], handlers[i]);
}

void messenger::dispatch(const message& msg)
{
  bool handled = false;
  if (!handlers.empty()) {
    for (std::size_t i = 0; i < handlers.size(); ++i) {
      if (handlers[i].first == msg.domain()) {
        while (handlers[i].first == msg.domain())
          handlers[i].second(msg);
        handled = true;
      } else if (handled)
        return;
    }
  }
  if (!handled)
    LOG(WARNING) << "Unhandled message with domain "
                 << std::size_t(msg.domain());
}

namespace detail
{
  void concurrent_messenger::subscribe(message::domain_type domain,
                                       msg_handler h)
  {
    {
      writer_lock lock(mx);
      messenger::subscribe(std::move(domain), std::move(h));
    }
  }

  void concurrent_messenger::dispatch(const message& msg)
  {
    {
      reader_lock lock(mx);
      messenger::dispatch(msg);
    }
  }
}
}
