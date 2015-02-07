#pragma once

#include <sma/schedule/blockrequestdesc.hpp>
#include <sma/nodeid.hpp>
#include <sma/util/hash.hpp>
#include <queue>
#include <vector>

namespace sma
{
class BlockRequestScheduler
{
public:

  BlockRequestScheduler (ForwardSchedulerImpl* ptr)
      : sched_ptr (ptr)
  {}

  void add_request_desc (const std::vector<BlockRequest>& requests)
  {
    // nodeID is neighbor's node, not origin node's ID.
    for (auto it = requests.begin(); it != requests.end(); ++it)
    {
      insert_request (*it);
    }
  }

  void sched()
  {
    fwd_requests(10); // 10 is the magic number. needs adjusting
  }

  int get_ttl (NodeID id, Hash content_name, size_t index)
  {
    auto requests_per_node = request_desc_table.find(id); 
    if (requests_per_node == request_desc_table.end())
      return -1;
    else
    {
      auto it = requests_per_node.begin();
      while (it != requests_per_node.end())
      {
        if (it->content_name == content_name
          && it->block_index == index)
        {
          auto current_time = sma::chrono::system_clock::now();
          auto ttl = it->expire_time;
          if (ttl > current_time)
          {
            return (ttl - current_time) / sched_ptr->get_sched_interval();
          }
          else
            return -1;
        }
        it++; 
      }
      return -1;
    }
  }

  double get_utility(NodeID id, Hash content_name, size_t index)
  {
    auto requests_per_node = request_desc_table.find(id);
    if (requests_per_node != request_desc_table.end())
    {
      auto requestIt = (requests_per_node->second).begin();
      auto current_time = sma::chrono::system_clock::now();
      double total_utility = 0;
      double target_utility = 0;
      while (requestIt != (requests_per_node->second).end())
      {
        if (requestIt->content_name == content_name 
          && requestIt->block_index == index)
        {
          if (requestIt->expire_time_point < current_time) 
          {
            (requests_per_node->second).erase(requestIt);
            return 0;
          }
          
          target_utility = requestIt->utility;
        }
        total_utility += requestIt->utility;
        requestIt++;
      }
      return target_utility / total_utility;
    }
    else
      return 0; 
  }

private:

  using block_request_desc_vec = std::vector<BlockRequestDesc>;
  std::unordered_map<NodeID, block_request_desc_vec> request_desc_table; 

  std::queue<BlockRequestDesc> request_queue;
  ForwardSchedulerImpl* sched_ptr;

  void fwd_request (int num_of_requests)
  {
    std::vector<BlockRequestDesc> request_to_fwd;
    while (!request_queue.empty() && num_of_requests > 0)
    {
      BlockRequestDesc desc = request_queue.front();
      request_to_fwd.push_back(desc);
      request_queue.pop();
      num_of_requests--;
    }
  
  }

  void insert_request (BlockRequest request)
  {

    request_queue.push(request);

    NodeID nodeID = request.get_node_from();
    auto requests_per_node = request_desc_table.find(nodeID);
    auto current_time = sma::chrono::system_clock::now();
    auto expire_time = currenttime 
        + std::chrono::duration_cast<std::chrono::milliseconds>(request->get_ttl()).count();

    BlockRequestDesc desc (request.get_content_name(),
                           request.get_index(),
                           request.get_utility(),
                           expire_time,
                           request.get_origin_location());


    if (requests_per_node == request_desc_table.end())
    {
      request_desc_table.emplace (nodeID, block_request_desc_vec);
      request_desc_table[nodeID].push_back (desc);
    }
    else
    {
        auto requestIt = (requests_per_node->second).begin();
        while (requestIt != (requests_per_node->second).end())
        {
          // prune outdated requests by the way

          if (requestIt->content_name == desc.content_name &&
                  requestIt->block_index == desc.block_index)
          {
            requestIt = (requests_per_node->second).erase(requestIt); 
          }
          else if (requestIt->expire_time_point < current_time)
          {
            requestIt = (requests_per_node->second).erase(requestIt);
          }
          else
          {
            ++requestIt; 
          }
        }
        (requests_per_node->second).push_back(desc);
    }
  }
};
}
