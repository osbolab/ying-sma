#pragma once

#include <sma/context.hpp>
#include <sma/messenger.hpp>
#include <sma/message.hpp>
#include <sma/async.hpp>

namespace sma
{
class Actor
{
  // clang-format off
  struct log_type {
    log_type(Context* ctx) : ctx(ctx) {
      i("\n\n---------------- session ---------------");
    }
    template <typename... Args> log_type const& t(Args&&... args) const;
    template <typename... Args> log_type const& d(Args&&... args) const;
    template <typename... Args> log_type const& i(Args&&... args) const;
    template <typename... Args> log_type const& w(Args&&... args) const;
    template <typename... Args> log_type const& e(Args&&... args) const;
    template <typename... Args> log_type const& f(Args&&... args) const;
  private:
    Context* ctx;
  };
  // clang-format on
public:
  virtual void dispose() = 0;
  virtual ~Actor();

  virtual void receive(Message const& msg) = 0;
  virtual void receive(Message const& msg, Actor* sender) = 0;

protected:
  Actor(Context* ctx);

  Context* context() const;
  NodeInfo const* this_node() const;

  // Messaging

  void subscribe(Message::Type type);
  void unsubscribe(Message::Type type);
  void post(Message const& msg);
  template <typename A>
  void post(Message const& msg);

  // Scheduling

  template <typename F, typename... A>
  Async::Task<F, A...> async(F&& f, A&&... args);

  log_type const log;

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

/******************************************************************************
 * Logging via the context's logger instance
 * This is where things need to change if you switch loggers.
 */
template <typename... Args>
Actor::log_type const& Actor::log_type::t(Args&&... args) const
{
  ctx->log()->trace(std::forward<Args>(args)...);
  return *this;
}
template <typename... Args>
Actor::log_type const& Actor::log_type::d(Args&&... args) const
{
  ctx->log()->debug(std::forward<Args>(args)...);
  return *this;
}
template <typename... Args>
Actor::log_type const& Actor::log_type::i(Args&&... args) const
{
  ctx->log()->info(std::forward<Args>(args)...);
  return *this;
}
template <typename... Args>
Actor::log_type const& Actor::log_type::w(Args&&... args) const
{
  ctx->log()->warn(std::forward<Args>(args)...);
  return *this;
}
template <typename... Args>
Actor::log_type const& Actor::log_type::e(Args&&... args) const
{
  ctx->log()->error(std::forward<Args>(args)...);
  return *this;
}
template <typename... Args>
Actor::log_type const& Actor::log_type::f(Args&&... args) const
{
  ctx->log()->fatal(std::forward<Args>(args)...);
  return *this;
}
}
