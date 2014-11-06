#include "messenger.hh"

#include <cstddef>
#include <unordered_map>


namespace sma
{

int Messenger::send_now(Message::Type type,
    const std::vector<Recipient>& recipients,
    const std::uint8_t* data,
    std::size_t len)
{
}

void Messenger::dispatch(const Message& msg)
{
  auto handler = handlers.find(msg.type);
  if (handler != handlers.end())
    handler->second(msg);
}
}
