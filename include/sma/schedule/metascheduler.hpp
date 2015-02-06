#pragma once

#include <sma/ccn/contentmetadata.hpp>

class MetaScheduler
{
public:
  void add_metadata (const std::vector<ContentMetadata*>& metadata)
  {}

  // max_entries_to_fwd should be set according to the data block size.
  void sched(int max_entries_to_fwd = 10)
  {
    // call func in ContentHelper 
  }
};
