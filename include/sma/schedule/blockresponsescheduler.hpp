#pragma once

#include <vector>
#include <utility>
#include <sma/util/hash.hpp>
#include <sma/neighbor.hpp>
#include <cassert>

class BlockResponseScheduler
{
public:
  
  BlockResponseScheduler (ForwardSchedulerImpl* ptr)
      : sched_ptr (ptr) 
  {}
    
  void add_responses(const std::pair<Hash, size_t>& blockid)
  {
      block_to_schedule.insert(blockid);
  }

  int sched()
  {
    int max_ttl = sched_ptr->get_max_ttl();
    int num_of_blocks = block_to_schedule.size();
    int storage = sched_ptr->get_storage();
    int bandwidth = sched_ptr->get_bandwidth();
    int num_of_neighbor = sched_ptr->get_num_of_neighbor();

    std::vector<std::vector<int>> ttls (num_of_neighbor,
            std::vector<int>(num_of_blocks), -1);

    std::vector<std::vector<double>> utils (num_of_neighbor,
            std::vector<double>(num_of_blocks, 0));


    //map blockid to seq from 0...n
    
    int seq = 0;
    auto it = block_to_schedule.begin();
    while (it != block_to_schedule.end())
    {
      block_to_seq.insert(*it, seq);

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

    
    int num_of_broadcasts = 0;

    for (int c=0; c<sched_result.size(); i++)
    {

      std::pair<Hash, size_t> block_id = get_blockid (c);
      if (sched_result[c][0] == 1)
      {
        //// freeze cache
        sched_ptr->freeze_block (block_id->first, 
                                 block_id->second);

        if (sched_result[c][1] == 0)
        {
          num_of_broadcasts++;
          sched_ptr->broadcast_block (block_id->first,
                                      block_id->second);
        }
      }
      else
      {
        //// unfreeze cache 
        sched_ptr->unfreeze_block (block_id->first,
                                   block_id->second);
      }
    }

    return num_of_broadcasts; // update the num_of_blocks to broadcast
    
  }

private:
  std::unordered_set<std::pair<Hash, size_t>> block_to_schedule;
  ForwardSchedulerImpl* sched_ptr;
  std::unordered_map<std::pair<Hash, size_t>, int> block_to_seq;

  std::pair<Hash, size_t> get_blockid (int seq)
  {
    auto blockIt = block_to_seq.begin();
    while (blockIt != block_to_seq.end())
    {
      if (blockIt->second == seq)
      {
        return blockIt->first; 
      }
      blockIt++; 
    }
    assert (blockIt != block_to_seq.end());
  }
};
