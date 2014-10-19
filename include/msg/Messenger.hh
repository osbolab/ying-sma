#pragma once

#include <memory>
#include <functional>

#include "Message.hh"


namespace sma
{

class Messenger
{
public:
  Message::Builder create();

  void post(std::unique_ptr<const Message> msg);

  void subscribe(Message::Type type, std::function<void(std::unique_ptr<Message>)> onMessage);
};

}