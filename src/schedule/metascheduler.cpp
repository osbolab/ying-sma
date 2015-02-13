#include <sma/schedule/metascheduler.hpp>
#include <cstddef>
#include <sma/schedule/forwardschedulerimpl.hpp>

namespace sma
{

    std::size_t MetaScheduler::sched()
    {
  	  return sched_ptr->fwd_metas();
    }

}