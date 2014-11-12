#include "messenger.hh"

#include "message.hh"
#include "channel.hh"
#include "rws_mutex.hh"
#include "log.hh"

#include <cstdint>


namespace sma
{

void Messenger::subscribe(Message::Type t, msg_handler h)
{
  {
    writer_lock lock(mx);
    // Sort the handlers by message type so all handlers for a particular
    // type are grouped together.
    handlers.emplace_back(std::move(t), std::move(h));
    for (auto i = handlers.size();
         i > 0 && handlers[i].first < handlers[i - 1].first;
         --i)
      std::swap(handlers[i - 1], handlers[i]);
  }
}

void Messenger::dispatch(const Message& msg)
{
  // LOCK handlers FOR READING
  reader_lock lock(mx);

  if (handlers.empty()) {
    LOG(WARNING) << "No message handlers; dropping type " << int(msg.type());
    return;
  }
  bool handled = false;
  const Message::Type t = msg.type();
  const std::size_t sz = handlers.size();
  // Handlers are executed IN THIS THREAD and IN THE CRITICAL SECTION
  for (std::size_t i = 0; i < sz; ++i) {
    if (handlers[i].first == t) {
      handlers[i].second(msg);
      handled = true;
    } else if (handled)
      return;
  }
  // UNLOCK handlers
  if (!handled)
    LOG(WARNING) << "Unhandled message type " << std::size_t{msg.type()};
}

int Messenger::send(Message builder)
{
  std::uint8_t buf[SEND_BUFFER_SIZE];
#if 0
  Message msg(std::move(builder), this_sender);
  if (msg.serialized_size() > SEND_BUFFER_SIZE)
    return -1;
  std::size_t wrote = msg.put_in(buf, SEND_BUFFER_SIZE) - buf;
  outbound->write(buf, wrote);
#endif

  return 0;
}
}
