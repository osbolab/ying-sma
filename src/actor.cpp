#include <sma/actor.hpp>
#include <sma/context.hpp>
#include <sma/message.hpp>
#include <sma/messenger.hpp>

#include <utility>


namespace sma
{
Actor::Actor(Context* context)
  : ctx(context)
{
  ctx->enter(this);
}
Actor::~Actor()
{
  ctx->msgr->unsubscribe(this);
  ctx->leave(this);
}

// clang-format off
void Actor::subscribe(Message::Type type) { ctx->msgr->subscribe(type, this); }
void Actor::unsubscribe(Message::Type type) { ctx->msgr->unsubscribe(type, this); }
void Actor::post(Message const& msg) { ctx->msgr->forward(msg); }
// clang-format on
}
