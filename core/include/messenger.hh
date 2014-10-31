#pragma once

#include "message.hh"


#include <memory>
#include <functional>


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