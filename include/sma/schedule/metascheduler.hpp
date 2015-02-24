#pragma once

#include <cstddef>
#include <sma/schedule/forwardschedulerimpl.hpp>
#include <cstdint>

namespace sma
{

class ForwardSchedulerImpl;

class MetaScheduler
{
public:

  MetaScheduler (ForwardSchedulerImpl* ptr)
		: sched_ptr (ptr)
  {}
	  
  std::uint16_t sched();
  
private:
	ForwardSchedulerImpl* sched_ptr;
};

}
