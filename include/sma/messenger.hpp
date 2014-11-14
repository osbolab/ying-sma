#pragma once

#include <sma/message.hpp>


namespace sma
{

class messenger
{
public:
  // The callback type for receiving incoming messages.
  using handler = std::function<void(const message&) >;

  virtual ~messenger() {}
  // Attach the given message handler callback to the message domain such that
  // all incoming messages with that domain are delivered to the handler.
  // Multiple handlers for a domain and duplicate handlers are acceptable.
  virtual messenger& subscribe(message::domain_type domain, handler h)
      = 0;
  // Construct a message from the given incomplete stub and post it to the
  // messenger's sink.
  virtual messenger& post(const message::stub& stub) = 0;
};
}
