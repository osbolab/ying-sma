#pragma once

#include <vector>
#include <utility>
#include <sma/util/hash.hpp>
#include <sma/neighbor.hpp>

class BlockResponseScheduler
{
public:
  
  BlockResponseScheduler (ForwardSchedulerImpl* ptr)
      : sched_ptr (ptr) 
  {}
    
  void add_responses(const std::pair<Hash, size_t>& blockid)
  {
    auto it = block_to_schedule.find (blockid);
    if (it != block_to_schedule.end())
    {
      block_to_schedule.insert(blockid);
    }
  }

  void sched()
  {
    int max_ttl = sched_ptr->get_max_ttl();
    int num_of_blocks = block_to_schedule.size();
    int storage = sched_ptr->get_storage();
    int bandwidth = sched_ptr->get_bandwidth();
    int num_of_neighbor = sched_ptr->get_num_of_neighbor();

    std::vector<std::vector<int>> ttls (num_of_neighbor,
            std::vector<int>(num_of_blocks));

    std::vector<std::vector<double>> utils (num_of_neighbor,
            std::vector<double>(num_of_blocks));


    //map blockid to seq from 0...n
    
    int seq = 0;
    auto it = block_to_schedule.begin();
    while (it != block_to_schedule.end())
    {
      block_to_schedule.insert(*it, seq);

      std::vector<Neighbor> neighbors = sched_ptr->get_neighbors();
      for (int i=0; i<neighbors.end(); i++)
      {
        NodeID node_id = neighbors[i].id;
        utils[i][seq] = sched_ptr->get_utility (node_id, 
                                               it->first->first, 
                                               it->first->second);
        ttls[i][seq] = sched_ptr->get_ttl (node_id,
                                           it->first->first,
                                           it->first->second);

      }
      seq++;
      it++; 
    }

    // t = 0...max_ttl+1
    std::vector<std::vector<int>> sched_result (num_of_blocks,
            std::vector<int>(max_ttl+2));


    LPSolver::solve (max_ttl,
                     num_of_blocks,
                     storage,
                     bandwidth,
                     num_of_neighbor,
                     ttls,
                     utils,
                     sched_result
                     );

    
    
  }

private:
  std::unordered_set<std::pair<Hash, size_t>> block_to_schedule;
  ForwardSchedulerImpl* sched_ptr;
  std::unordered_map<std::pair<Hash, size_t>, int> block_to_seq;
};
