#pragma once

#include <sma/schedule/blockrequestdesc.hpp>
#include <sma/ccn/blockrequestargs.hpp>
#include <sma/nodeid.hpp>
#include <sma/util/hash.hpp>
#include <queue>
#include <vector>
#include <cstddef>
#include <sma/ccn/blockindex.hpp> 

namespace sma
{
class BlockRequestScheduler
{
public:

  BlockRequestScheduler (ForwardSchedulerImpl* ptr)
      : sched_ptr (ptr)
  {}

  void add_request_desc (const std::vector<BlockRequestArgs>& requests);
  std::size_t sched();
  int get_ttl (NodeID id, Hash content_name, BlockIndex block_index);
  double get_utility(NodeID id, Hash content_name, BlockIndex block_index);

private:

  std::unordered_map<NodeID, std::vector<BlockRequestDesc>> request_desc_table; 
  std::queue<BlockRequestDesc> request_queue;
  ForwardSchedulerImpl* sched_ptr;

  int fwd_request (int max_num_of_requests);
  void insert_request (BlockRequestArgs request);
  
};
}
