#include <sma/actor.hpp>
#include <sma/messenger.hpp>


namespace sma
{
Actor::Actor(Context* context)
  : ctx(context)
  , log(context->log())
{
}
Actor::~Actor()
{
  ctx->msgr->unsubscribe(this);
  ctx->async->purge_events_for(this);
}

// clang-format off
void Actor::subscribe(MessageType type) { ctx->msgr->subscribe(type, this); }
void Actor::unsubscribe(MessageType type) { ctx->msgr->unsubscribe(type, this); }
void Actor::post(Message msg) { ctx->msgr->forward(std::move(msg)); }
// clang-format on
}
