#pragma once

#include <sma/detail/message_type.hpp>


namespace sma
{

class actor;
struct message;

class messenger
{
public:
  virtual ~messenger() {};

  virtual messenger& subscribe(message_type type, actor* subscriber) = 0;
  virtual messenger& unsubscribe(message_type type, actor* subscriber) = 0;
  virtual messenger& unsubscribe(actor* subscriber) = 0;

  virtual messenger& post(message const& msg) = 0;
};
}
