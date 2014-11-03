#pragma once

#include "message.hh"


#include <memory>
#include <functional>


namespace sma
{

class Messenger
{
public:
  using Handler = std::function<bool(const Message*)>;

  // Blocks the caller
  int send_now(Type type,
               const Address* recipients,
               const std::uint8_t* data,
               std::size_t len);

  void subscribe(Message::Type type, Handler on_message);
};
}
