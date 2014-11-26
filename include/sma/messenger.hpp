#pragma once

#include <sma/message.hpp>


namespace sma
{

class Actor;

class Messenger
{
public:
  virtual ~Messenger() {};

  virtual Messenger& subscribe(MessageType type, Actor* subscriber) = 0;
  virtual Messenger& unsubscribe(MessageType type, Actor* subscriber) = 0;
  virtual Messenger& unsubscribe(Actor* subscriber) = 0;

  virtual Messenger& forward(Message&& msg) = 0;
};
}
