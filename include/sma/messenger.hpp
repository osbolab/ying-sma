#pragma once

#include "detail/message_type.hpp"


namespace sma
{

class actor;
struct message;

class messenger
{
public:
  virtual ~messenger() {}
  // Attach the given message handler callback to the message type such that
  // all incoming messages with that type are delivered to the handler.
  // Multiple handlers for a type and duplicate handlers are acceptable.
  virtual messenger& subscribe(message_type type, actor* subscriber) = 0;
  // Construct a message from the given incomplete stub and post it to the
  // messenger's sink.
  virtual messenger& post(message const& msg) = 0;
};
}
