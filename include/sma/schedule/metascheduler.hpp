#pragma once

#include <cstddef>
#include <sma/schedule/forwardschedulerimpl.hpp>

namespace sma
{
	
class MetaScheduler
{
public:

  MetaScheduler (ForwardSchedulerImpl* ptr)
		: sched_ptr (ptr)
  {}
	  
  std::size_t sched();
  
private:
	ForwardSchedulerImpl* sched_ptr;
};

}
