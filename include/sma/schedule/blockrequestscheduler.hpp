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
#include <unordered_set>
#include <cstdint>

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
  std::uint16_t sched();
  int get_ttl (NodeId id, Hash content_name, BlockIndex block_index);
  float get_utility(NodeId id, Hash content_name, BlockIndex block_index);
  void clear_request (Hash hash, BlockIndex index);
  void delete_request_from_node (NodeId id, Hash hash, BlockIndex index);
  std::unordered_set<NodeId> get_request_nodes() const;
  bool has_request_for_block (BlockRef block) const;
  std::vector<BlockRequestDesc> get_requests_for_block (BlockRef block) const;

private:

  using clock = std::chrono::system_clock;
  using Ms = std::chrono::milliseconds;

  std::unordered_map<NodeId, std::vector<BlockRequestDesc>> request_desc_table; 
  std::queue<BlockRequestDesc> request_queue;
  ForwardSchedulerImpl* sched_ptr;
//  std::unordered_map<BlockRef, clock::time_point> forwarded_requests; 

  std::uint16_t fwd_requests (std::size_t max_num_of_requests);
  void insert_request (NodeId id, BlockRequestArgs request);
//  void refresh_forwarded_requests();
  
};

}