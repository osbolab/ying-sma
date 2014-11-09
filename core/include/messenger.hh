#pragma once

#include "actor.hh"
#include "message.hh"
#include "channel.hh"
#include "rws_mutex.hh"

#include <cstdint>
#include <functional>


namespace sma
{

class Messenger
{

public:
  Messenger(ActorId this_sender, Channel* outbound)
    : outbound(outbound)
    , this_sender(this_sender)
  {
  }

  virtual ~Messenger() {}

  using MessageHandler = std::function<void(const Message&) >;

  Messenger(Messenger&& o) = default;
  Messenger& operator=(Messenger&& o) = default;

  // Does not block
  // handler may be called concurrently from any number of threads.
  virtual void subscribe(Message::Type type, MessageHandler handler);

  virtual void dispatch(const Message& msg);

  // Blocks the caller
  // Returns 0 on success
  virtual int send(Message builder);

protected:
  struct Wrapper {
    Wrapper(Message::Type type, MessageHandler handler)
      : msgtype(std::move(type))
      , handler(std::move(handler))
    {
    }
    Message::Type msgtype;
    MessageHandler handler;
  };

  ActorId this_sender;
  Channel* outbound;
  std::vector<Wrapper> handlers;
  rws_mutex mx;


private:
  static const std::size_t SEND_BUFFER_SIZE = 1024;
};
}
