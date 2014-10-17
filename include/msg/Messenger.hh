#pragma once

#include <memory>
#include <functional>

#include "Message.hh"
#include "MessageBuilder.hh"
#include "MessageFilter.hh"


namespace sma
{

class Messenger
{
public:
  MessageBuilder create();
  
  void post(std::unique_ptr<const Message> msg);

  void subscribe(Message::Type type, std::function<void(std::unique_ptr<Message>)> onMessage);
};

}