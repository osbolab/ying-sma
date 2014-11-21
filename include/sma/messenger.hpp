#pragma once

#include <sma/message.hpp>


namespace sma
{

class Actor;

class Messenger
{
public:
  virtual ~Messenger() {};

  virtual Messenger& subscribe(Message::Type type, Actor* subscriber) = 0;
  virtual Messenger& unsubscribe(Message::Type type, Actor* subscriber) = 0;
  virtual Messenger& unsubscribe(Actor* subscriber) = 0;

  virtual Messenger& post(Message const& msg) = 0;
};
}
