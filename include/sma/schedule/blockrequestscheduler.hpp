#pragma once

#include <sma/schedule/blockrequestdesc.hpp>
#include <sma/ccn/blockrequestargs.hpp>
#include <sma/nodeid.hpp>
#include <sma/util/hash.hpp>
#include <queue>
#include <vector>
#include <cstddef>
#include <sma/schedule/forwardschedulerimpl.hpp>
#include <sma/ccn/ccnnode.hpp>
#include <unordered_map>
#include <sma/ccn/blockref.hpp> 

namespace sma
{

class ForwardSchedulerImpl;

class BlockRequestScheduler
{
public:

  BlockRequestScheduler (ForwardSchedulerImpl* ptr)
      : sched_ptr (ptr)
  {}

  void add_requests (NodeId id, std::vector<BlockRequestArgs> requests);
  std::size_t sched();
  int get_ttl (NodeId id, Hash content_name, BlockIndex block_index);
  float get_utility(NodeId id, Hash content_name, BlockIndex block_index);

private:

  std::unordered_map<NodeId, std::vector<BlockRequestDesc>> request_desc_table; 
  std::queue<BlockRequestDesc> request_queue;
  ForwardSchedulerImpl* sched_ptr;

  std::size_t fwd_requests (std::size_t max_num_of_requests);
  void insert_request (NodeId id, BlockRequestArgs request);
  
};

}
