#include <sma/schedule/blockrequestscheduler.hpp>
#include <cstddef>
#include <sma/schedule/blockrequestdesc.hpp>
#include <sma/ccn/blockrequestargs.hpp>
#include <sma/nodeid.hpp>
#include <sma/util/hash.hpp>
#include <queue>
#include <vector>

namespace sma
{
    void BlockRequestScheduler::add_request_desc (const std::vector<BlockRequestArgs>& requests)
    {
      // nodeID is neighbor's node, not origin node's ID.
      for (auto it = requests.begin(); it != requests.end(); ++it)
      {
        insert_request (*it);
      }
    }
	
	std::size_t BlockRequestScheduler::sched()
	{
	  return fwd_requests(10); // 10 is the magic number. needs adjusting
	}
	
    int BlockRequestScheduler::get_ttl (NodeId id, Hash content_name, BlockIndex block_index)
    {
      auto requests_per_node = request_desc_table.find(id); 
      if (requests_per_node == request_desc_table.end())
        return -1;
      else
      {
        auto it = (requests_per_node->second).begin();
        while (it != (requests_per_node->second).end())
        {
          if (it->content_name == content_name
            && it->block_index == block_index)
          {
            auto current_time = sma::chrono::system_clock::now();
            auto deadline = it->expire_time;
            if (deadline > current_time)
            {
  			int time_span = std::chrono::duration_cast<std::chrono::microseconds>(deadline 
  				- current_time).count();
              return time_span / sched_ptr->get_sched_interval();
            }
            else
              return -1;
          }
          it++; 
        }
        return -1;
      }
    }
	
    float BlockRequestScheduler::get_utility(NodeId id, Hash content_name, BlockIndex block_index)
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
            && requestIt->block_index == block_index)
          {
            if (requestIt->expire_time < current_time) 
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
	
    std::size_t BlockRequestScheduler::fwd_requests (std::size_t max_num_of_requests)
    {
      std::vector<BlockRequestArgs> request_to_fwd;
      while (!request_queue.empty() && max_num_of_requests > 0)
      {
        BlockRequestDesc desc = request_queue.front();
      
        // translate from BlockRequestDesc to BlockRequestArgs, all about the ttl
        auto current_time = sma::chrono::system_clock::now();
        if (desc.expire_time < current_time)  continue;
  	    auto ttl = std::chrono::duration_cast<std::chrono::milliseconds>
            (desc.expire_time - current_time);
        auto arg 
            = BlockRequestArgs (desc.content_name,
                              desc.block_index,
                              desc.utility,
                              ttl,
  							  desc.requester,
                              desc.origin_location);
     
        request_to_fwd.push_back(arg);
        request_queue.pop();
        max_num_of_requests--;
      }

      if (request_to_fwd.size() > 0)
        sched_ptr->request_blocks (request_to_fwd);

      return request_to_fwd.size();
    }
	
    void BlockRequestScheduler::insert_request (BlockRequestArgs request)
    {
      NodeId nodeID = request.requester;
      auto requests_per_node = request_desc_table.find(nodeID);
      auto current_time = sma::chrono::system_clock::now();
	
  	// change from relative ttl to absolute ttl locally
      auto expire_time = current_time + request.ttl<std::chrono::microseconds>();

      BlockRequestDesc desc (request.hash,
                             request.index,
                             request.utility,
                             expire_time,
                             request.requester,
                             request.requester_position);

      request_queue.push(desc);

      if (requests_per_node == request_desc_table.end())
      {
        request_desc_table.emplace (nodeID, 
                std::vector<BlockRequestDesc>());
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
            else if (requestIt->expire_time < current_time)
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
}
