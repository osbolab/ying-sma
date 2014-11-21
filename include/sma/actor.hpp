#pragma once

#include <sma/context.hpp>
#include <sma/message.hpp>


namespace sma
{
struct message;

class actor
{
public:
  actor(context ctx);
  virtual ~actor();

  virtual void on_message(message const& msg) = 0;

protected:
  void subscribe(message_type type);
  void unsubscribe(message_type type);
  void post(message const& msg);

  context ctx;
};
}
