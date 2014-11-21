#include <sma/actor.hpp>
#include <sma/context.hpp>
#include <sma/message.hpp>
#include <sma/messenger.hpp>

#include <utility>


namespace sma
{
actor::actor(context ctx)
  : ctx(std::move(ctx))
{
}

actor::~actor()
{
  ctx.msgr->unsubscribe(this);
}

void actor::subscribe(message_type type)
{
  ctx.msgr->subscribe(type, this);
}

void actor::unsubscribe(message_type type)
{
  ctx.msgr->unsubscribe(type, this);
}

void actor::post(message const& msg)
{
  ctx.msgr->post(msg);
}

}
