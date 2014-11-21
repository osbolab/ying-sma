#include <sma/actor.hpp>
#include <sma/context.hpp>
#include <sma/message.hpp>
#include <sma/messenger.hpp>

#include <utility>


namespace sma
{
Actor::Actor(Context context)
  : ctx(std::move(context))
{
  ctx.enter(this);
}
Actor::~Actor()
{
  ctx.msgr->unsubscribe(this);
  ctx.leave(this);
}

void Actor::subscribe(Message::Type type) { ctx.msgr->subscribe(type, this); }
void Actor::unsubscribe(Message::Type type)
{
  ctx.msgr->unsubscribe(type, this);
}
void Actor::post(message const& msg) { ctx.msgr->forward(msg); }
}
