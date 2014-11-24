#include <sma/actor.hpp>
#include <sma/context.hpp>
#include <sma/message.hpp>
#include <sma/messenger.hpp>

#include <utility>


namespace sma
{
Actor::Actor(Context* context)
  : ctx(context)
  , log(context)
{
  ctx->enter(this);
}
Actor::~Actor()
{
  ctx->msgr->unsubscribe(this);
  ctx->leave(this);
}

Context* Actor::context() const { return ctx; }
NodeInfo const* Actor::this_node() const { return ctx->this_node(); }

// clang-format off
void Actor::subscribe(Message::Type type) { ctx->msgr->subscribe(type, this); }
void Actor::unsubscribe(Message::Type type) { ctx->msgr->unsubscribe(type, this); }
void Actor::post(Message const& msg) { ctx->msgr->forward(msg); }
// clang-format on
}
