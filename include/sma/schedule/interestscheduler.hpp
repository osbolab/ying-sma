#pragma once

#include <sma/ccn/interest.hpp>

class InterestScheduler
{
public:

  // max_entries_to_fwd should be set according to the data block size.
  void sched(int max_entries_to_fwd = 10) 
  {
    // call announce() func in InterestHelperImpl 
  }
};
