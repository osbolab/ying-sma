#include <sma/schedule/blockresponsescheduler.hpp>
#include <vector>
#include <utility>
#include <sma/util/hash.hpp>
#include <sma/neighbor.hpp>
#include <cassert>
#include <sma/ccn/blockindex.hpp> 
#include <cstddef>

namespace sma
{
    void BlockResponseScheduler::add_responses(const std::pair<Hash, BlockIndex>& blockid)
    {
        block_to_schedule.insert(blockid);
    }
	
    std::size_t BlockResponseScheduler::sched()
    {
      std::size_t num_of_blocks = block_to_schedule.size();
      if (num_of_blocks == 0)
          return 0;
      std::size_t max_ttl = sched_ptr->get_max_ttl();
      std::size_t storage = sched_ptr->get_storage();
      std::size_t bandwidth = sched_ptr->get_bandwidth();
      std::size_t num_of_neighbor = sched_ptr->get_num_of_neighbor();

      std::vector<std::vector<int>> ttls (num_of_neighbor,
              std::vector<int>(num_of_blocks), -1);

      std::vector<std::vector<float>> utils (num_of_neighbor,
              std::vector<float>(num_of_blocks, 0));


      //map blockid to seq from 0...n
    
      std::size_t seq = 0;
      auto it = block_to_schedule.begin();
      while (it != block_to_schedule.end())
      {
        block_to_seq.insert(*it, seq);

        std::vector<Neighbor> neighbors = sched_ptr->get_neighbors();
        for (std::size_t i=0; i<neighbors.end(); i++)
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
      std::vector<std::vector<std::size_t>> sched_result (num_of_blocks,
              std::vector<std::size_t>(max_ttl+2));


      LPSolver::solve (max_ttl,
                       num_of_blocks,
                       storage,
                       bandwidth,
                       num_of_neighbor,
                       ttls,
                       utils,
                       sched_result
                       );
	  
	  std::vector<std::pair<Hash, BlockIndex>> blocks_to_freeze;
	  std::vector<std::pair<Hash, BlockIndex>> blocks_to_unfreeze;
	  std::vector<std::pair<Hash, BlockIndex>> blocks_to_broadcast;

      for (std::size_t c=0; c<sched_result.size(); c++)
      {

        std::pair<Hash, BlockIndex> block_id = get_blockid (c);
        if (sched_result[c][0] == 1)
        {
          //// freeze cache
		  blocks_to_freeze.push_back (block_id);

          if (sched_result[c][1] == 0)
          {
            sched_ptr->broadcast_block (block_id->first,
                                        block_id->second);
			blocks_to_broadcast.push_back (block_id);
  		    block_to_schedule.erase (block_id);
          }
        }
        else
        {
          //// unfreeze cache 
          blocks_to_unfreeze.push_back (block_id);
        }
      }
	  
	  sched_ptr->freeze_blocks (blocks_to_freeze);
	  sched_ptr->unfreeze_blocks (blocks_to_unfreeze);
	  
	  for (std::size_t c=blocks_to_broadcast.begin(); c!=blocks_to_broadcast.end(); c++)
	  {
		sched_ptr->broadcast_block (c->first, c->second);
	  }

      return blocks_to_broadcast.size(); // update the num_of_blocks to broadcast  
    }
	
    std::pair<Hash, BlockIndex> BlockResponseScheduler::get_blockid (std::size_t seq)
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
}
