#include "messenger.hh"
#include "rws_mutex.hh"
#include "message.hh"
#include "channel.hh"
#include "log.hh"

#include <cstdint>


namespace sma
{

void Messenger::subscribe(Message::Type type, MessageHandler handler)
{
  {
    writer_lock lock(mx);
    handlers.emplace_back(type, std::move(handler));
  }
  LOG(DEBUG) << "Subscribed message type " << std::size_t{type};
}

void Messenger::dispatch(const Message& msg)
{
  MessageHandler handler;

  {
    reader_lock lock(mx);
    for (auto& w : handlers)
      if (w.msgtype == msg.type())
        handler = w.handler;
  }

  if (handler)
    handler(msg);
  else
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
