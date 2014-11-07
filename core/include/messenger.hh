#pragma once

#include "message.hh"
#include "channel.hh"

#include <cstddef>
#include <mutex>
#include <functional>
#include <unordered_map>


namespace sma
{

class Messenger
{
  using Lock = std::unique_lock<std::mutex>;

public:
  Messenger(Message::Address this_sender, Channel* outbound)
    : outbound(outbound)
    , this_sender(this_sender)
  {
  }

  virtual ~Messenger() {}

  using MessageHandler = std::function<void(const Message&) >;

  Messenger(Messenger&& o) = default;
  Messenger& operator=(Messenger&& o) = default;

  // Does not block, but on_message may be called in multiple threads
  // concurrently.
  virtual void subscribe(Message::Type type, MessageHandler handler);

  virtual void dispatch(const Message& msg);

  // Blocks the caller
  // Returns 0 on success
  virtual int send(Message::Builder builder);

protected:
  Message::Address this_sender;
  Channel* outbound;
  std::mutex mutex;
  std::unordered_map<Message::Type, MessageHandler> handlers;

private:
  static const std::size_t SEND_BUFFER_SIZE = 1024;
};
}
