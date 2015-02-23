#pragma once

#include <vector>
#include <utility>
#include <sma/util/hash.hpp>
#include <sma/neighbor.hpp>
#include <cassert>
#include <sma/ccn/blockref.hpp>
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
    
  void add_response(NodeId id, BlockRef blockid);
  std::size_t sched();

private:
  std::unordered_set<BlockRef> block_to_schedule;
  std::unordered_set<BlockRef> stored_block_to_schedule;
  std::unordered_set<BlockRef> block_arrived_buf;
  ForwardSchedulerImpl* sched_ptr;
  std::unordered_map<BlockRef, std::size_t> block_to_seq;

  BlockRef get_blockid (std::size_t seq);
  
};

}
