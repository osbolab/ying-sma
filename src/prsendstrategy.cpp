#include <sma/prsendstrategy.hpp>
#include <sma/linklayer.hpp>
#include <sma/context.hpp>

#include <sma/async.hpp>

namespace sma
{
PrSendStrategy::PrSendStrategy(Context& context)
  : SendStrategy(*context.linklayer)
  , context(&context)
  , distribute(0, 1)
{
}

void PrSendStrategy::notify() { schedule_timeslot(); }

void PrSendStrategy::schedule_timeslot()
{
  Lock lock(mx);
  if (is_scheduled)
    return;

  asynctask(&PrSendStrategy::do_timeslot, this).do_in(timestep);
  is_scheduled = true;
}

void PrSendStrategy::do_timeslot()
{
  Lock lock(mx);
  is_scheduled = false;

  auto prob = distribute(context->prng);

  if (prob >= threshold_min && prob <= threshold_max)
    if (!llayer->forward_one())
      return;

  schedule_timeslot();
}
}
