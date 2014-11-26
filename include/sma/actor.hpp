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

  Context* context() const { return ctx; }

  // Messaging

  virtual void receive(Message msg) = 0;

  void post(Message msg);

  // Scheduling

  template <typename F, typename... A>
  Async::Task<F, A...> async(F&& f, A&&... args);

protected:
  Actor(Context* context);

  void subscribe(MessageType type);
  void unsubscribe(MessageType type);

  Logger log;

private:
  Context* ctx;
};

template <typename F, typename... A>
Async::Task<F, A...> Actor::async(F&& f, A&&... args)
{
  return ctx->async->make_task(
      this, std::forward<F>(f), std::forward<A>(args)...);
}
}
