#pragma once

#include <sma/context.hpp>
#include <sma/messenger.hpp>
#include <sma/message.hpp>
#include <sma/async.hpp>


namespace sma
{
class Actor
{
public:
  virtual void dispose() = 0;
  virtual ~Actor();

  virtual void receive(Message const& msg) = 0;
  virtual void receive(Message const& msg, Actor* sender) = 0;

protected:
  Actor(Context* ctx);

  // Messaging

  void subscribe(Message::Type type);
  void unsubscribe(Message::Type type);
  void post(Message const& msg);
  template <typename A>
  void post(Message const& msg);

  // Scheduling

  template <typename F, typename... A>
  Async::Task<F, A...> async(F&& f, A&&... args);

private:
  Context* ctx;
};


template <typename A>
void Actor::post(Message const& msg)
{
  ctx->get_actor_by_type<A>()->receive(msg, this);
}

template <typename F, typename... A>
Async::Task<F, A...> Actor::async(F&& f, A&&... args)
{
  return ctx->async->make_task(std::forward<F>(f), std::forward<A>(args)...);
}
}
