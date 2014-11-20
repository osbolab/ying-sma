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
}

void actor::unsubscribe(message_type type)
{
}

void actor::post(message const& msg)
{
}

}
