#pragma once

#include <vector>
#include <utility>
#include <sma/util/hash.hpp>
#include <sma/neighbor.hpp>
#include <cassert>
#include <sma/ccn/blockindex.hpp> 
#include <unordered_set>
#include <unordered_map>
#include <sma/ccn/contenthelper.hpp>

namespace sma
{

class ForwardSchedulerImpl;

class BlockResponseScheduler
{
public:
  
  BlockResponseScheduler (ForwardSchedulerImpl* ptr)
      : sched_ptr (ptr) 
  {}
    
  void add_responses(const std::pair<Hash, BlockIndex>& blockid);
  std::size_t sched();

private:
  std::unordered_set<std::pair<Hash, BlockIndex>> block_to_schedule;
  ForwardSchedulerImpl* sched_ptr;
  std::unordered_map<std::pair<Hash, BlockIndex>, std::size_t> block_to_seq;

  std::pair<Hash, BlockIndex> get_blockid (std::size_t seq);
  
};

}
