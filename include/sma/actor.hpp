#pragma once

#include <sma/context.hpp>
#include <sma/messagetype.hpp>
#include <sma/async.hpp>
#include <sma/io/log>

namespace sma
{
struct Message;

class Actor
{
public:
  virtual ~Actor();
  virtual void dispose() = 0;

  virtual void receive(Message const& msg) = 0;

protected:
  Actor(Context* context);

  // Messaging

  void subscribe(MessageType type);
  void unsubscribe(MessageType type);
  void post(Message const& msg);

  // Scheduling

  template <typename F, typename... A>
  Async::Task<F, A...> async(F&& f, A&&... args);

  Logger const log;
  Context* context;
};

template <typename F, typename... A>
Async::Task<F, A...> Actor::async(F&& f, A&&... args)
{
  return context->async->make_task(std::forward<F>(f), std::forward<A>(args)...);
}
}
