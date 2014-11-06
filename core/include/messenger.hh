#pragma once

#include "message.hh"


#include <cstddef>
#include <functional>
#include <unordered_map>


namespace sma
{

class Messenger
{
public:
  virtual ~Messenger() {}

  using MessageHandler = std::function<bool(const Message&) >;

  Messenger(Messenger&& o) = default;
  Messenger& operator=(Messenger&& o) = default;


  // Blocks the caller
  // Returns 0 on success
  virtual int send_now(Message::Type type,
               const std::vector<Address>& recipients,
               const std::uint8_t* data,
               std::size_t len);

  // Does not block, but on_message may be called in multiple threads
  // concurrently.
  virtual void subscribe(Message::Type type, MessageHandler on_message);

  virtual void dispatch(const Message& msg);

protected:
  std::unordered_map<Message::Type, MessageCallback> handlers;
};
}
