#include <sma/actor.hpp>
#include <sma/messenger.hpp>


namespace sma
{
Actor::Actor(Context* context)
  : context(context)
  , log(context->log())
{
}
Actor::~Actor()
{
  context->msgr->unsubscribe(this);
}

// clang-format off
void Actor::subscribe(MessageType type) { context->msgr->subscribe(type, this); }
void Actor::unsubscribe(MessageType type) { context->msgr->unsubscribe(type, this); }
void Actor::post(Message const& msg) { context->msgr->forward(msg); }
// clang-format on
}
