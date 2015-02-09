#include <sma/schedule/interestscheduler.hpp>
#include <cstddef>
#include <sma/schedule/forwardschedulerimpl.hpp>

namespace sma
{
	
    std::size_t InterestScheduler::sched() 
    {
  	  return sched_ptr->fwd_interests();
    }

}
