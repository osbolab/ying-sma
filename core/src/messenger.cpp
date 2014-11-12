#include <sma/core/messenger.hpp>
#include <sma/core/message.hpp>
#include <sma/core/channel.hpp>
#include <sma/core/rws_mutex.hpp>
#include <sma/log.hpp>

#include <cstdint>


namespace sma
{

void messenger::subscribe(message::domain_type domain, msg_handler h)
{
  {
    writer_lock lock(mx);

    // We could use a binary search to find the insertion point, but
    // it still then requires an O(n) step to create the gap, so
    // we'd have an O(log) search and an O(n) insert.
    // Instead just do them at the same time by sliding in the back (heh)

    handlers.emplace_back(std::move(domain), std::move(h));
    auto i = handlers.size();
    while (--i > 0 && handlers[i].first < handlers[i - 1].first)
      std::swap(handlers[i - 1], handlers[i]);
  }
}

void messenger::dispatch(const message& msg)
{
  bool handled = false;
  {    // LOCK handlers FOR READING
    reader_lock lock(mx);

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
  }    // UNLOCK handlers
  if (!handled)
    LOG(WARNING) << "Unhandled message with domain "
                 << std::size_t(msg.domain());
}

int messenger::send(message builder)
{
  std::uint8_t buf[SEND_BUFFER_SIZE];
#if 0
  message msg(std::move(builder), this_sender);
  if (msg.serialized_size() > SEND_BUFFER_SIZE)
    return -1;
  std::size_t wrote = msg.put_in(buf, SEND_BUFFER_SIZE) - buf;
  outbound->write(buf, wrote);
#endif

  return 0;
}
}
