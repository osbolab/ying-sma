#pragma once

#include <sma/schedule/blockrequest.hpp>
#include <sma/nodeid.hpp>
#include <sma/util/hash.hpp>

namespace sma
{
class BlockRequestScheduler
{
public:

  void add_requests (const std::vector<BlockRequest*>& requests)
  {
    // nodeID is neighbor's node, not origin node's ID.
    for (auto it = requests.begin(); it != requests.end(); ++it)
    {
      insert_request (*it);
    }
  }

  void sched()
  {
    // call asynctask?
  }

private:

  std::unordered_map<NodeID, std::vector<BlockRequest*>> requestTable; 
  
  void insert_request (BlockRequest* request)
  {
    NodeID nodeID = request->get_node_id_from();
    auto requests_per_node = requestTable.find(nodeID);
    if (requests_per_node == requestTable.end())
    {
      requestTable.emplace (nodeID, std::vector<BlockRequest*>());
      requestTable[nodeID].push_back (request);
    }
    else
    {
        auto requestIt = (requests_per_node->second).begin();
        while (requestIt != (requests_per_node->second).end())
        {
          if (requestIt->get_name() == request->get_name() &&
                  requestIt->get_index() == request->get_index())
          {
            requestIt = (requests_per_node->second).erase(requestIt); 
          }
          else
          {
            ++requestIt; 
          }
        }
        (requests_per_node->second).push_back(request);
    }
  }

  // for test use
};
}
