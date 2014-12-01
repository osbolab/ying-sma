#include <sma/prforwardstrategy.hpp>
#include <sma/linklayer.hpp>
#include <sma/context.hpp>

#include <sma/async.hpp>

namespace sma
{
PrForwardStrategy::PrForwardStrategy(Context& context)
  : ForwardStrategy(*context.linklayer)
  , context(&context)
  , distribute(0, 1)
{
}

void PrForwardStrategy::notify() { schedule_timeslot(); }

void PrForwardStrategy::schedule_timeslot()
{
  Lock lock(mx);
  if (is_scheduled)
    return;

  asynctask(&PrForwardStrategy::do_timeslot, this).do_in(timestep);
  is_scheduled = true;
}

void PrForwardStrategy::do_timeslot()
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
