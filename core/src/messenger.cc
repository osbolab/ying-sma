#include "messenger.hh"
#include "message.hh"
#include "channel.hh"
#include "log.hh"

#include <cstddef>
#include <unordered_map>


namespace sma
{

void Messenger::subscribe(Message::Type type, MessageHandler handler)
{
  Lock lock(mutex);
  handlers[type] = std::move(handler);
  LOG(DEBUG) << "Subscribed message type " << std::size_t{type};
}

void Messenger::dispatch(const Message& msg)
{
  MessageHandler handler;
  {
    Lock lock(mutex);
    auto subscription = handlers.find(msg.type());
    if (subscription != handlers.end())
      handler = subscription->second;
    else {
      LOG(WARNING) << "Unhandled message type " << std::size_t{msg.type()};
      return;
    }
  }
  handler(msg);
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
