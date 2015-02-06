#pragma once

#include <sma/ccn/interest.hpp>

class InterestScheduler
{
public:

  void add_interests (const std::vector<Interest>& interests)
  {}

  // max_entries_to_fwd should be set according to the data block size.
  void sched(int max_entries_to_fwd = 10) 
  {
    // call announce() func in InterestHelperImpl 
  }
};
