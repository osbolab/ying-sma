#pragma once

#include <sma/schedule/BlockResponse.hpp>
#include <vector>

class BlockResponseScheduler
{
public:
 
  void add_responses(const std::vector<BlockResponse*>& responses)
  {
    for (auto it = responses.begin(); it != responses.end(); it++)
        block_to_schedule
  }

private:
  std::unordered_set<BlockResponse*> block_to_schedule;
  ForwardSchedulerImpl* sched_ptr;
};
