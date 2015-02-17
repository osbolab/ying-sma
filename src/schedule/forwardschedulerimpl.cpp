#include <sma/schedule/forwardscheduler.hpp>
#include <sma/schedule/forwardschedulerimpl.hpp>
#include <sma/ccn/ccnnode.hpp>
#include <sma/ccn/blockref.hpp>
#include <sma/schedule/interestscheduler.hpp>
#include <sma/schedule/metascheduler.hpp>
#include <sma/schedule/blockrequestscheduler.hpp>
#include <sma/schedule/blockresponsescheduler.hpp>
#include <sma/neighbor.hpp>
#include <cstddef>
#include <sma/ccn/blockrequestargs.hpp>
#include <vector>
#include <sma/neighborhelper.hpp>
#include <sma/ccn/contenthelper.hpp>
#include <sma/ccn/interesthelper.hpp>
#include <sma/schedule/lpsolver.hpp>
#include <sma/async.hpp>
#include <chrono>
#include <sma/util/event.hpp>
#include <sma/io/log>

namespace sma
{

    using namespace std::placeholders;

    ForwardSchedulerImpl::ForwardSchedulerImpl(CcnNode& host_node, std::uint32_t interval)
        : ForwardScheduler (host_node, interval)
    {
      interest_sched_ptr = new InterestScheduler(this);
      meta_sched_ptr = new MetaScheduler(this);
      blockrequest_sched_ptr = new BlockRequestScheduler(this);
      blockresponse_sched_ptr = new BlockResponseScheduler(this);

      node.content->on_block_arrived() += std::bind(&ForwardSchedulerImpl::on_block, this, _1);
      node.content->on_blocks_requested() += std::bind(&ForwardSchedulerImpl::on_blockrequest, this, _1, _2);

      // add time_out async callback later


      asynctask (&ForwardSchedulerImpl::sched, this).do_in (
              std::chrono::milliseconds (ForwardScheduler::get_sched_interval()));
    }

    ForwardSchedulerImpl::~ForwardSchedulerImpl()
    {
      if (interest_sched_ptr != NULL)  delete interest_sched_ptr;
      if (meta_sched_ptr != NULL)  delete meta_sched_ptr;
      if (blockrequest_sched_ptr != NULL)  delete blockrequest_sched_ptr;
      if (blockresponse_sched_ptr != NULL)  delete blockresponse_sched_ptr;
    }

    bool ForwardSchedulerImpl::on_blockrequest (NodeId id, std::vector<BlockRequestArgs> requests)
    {
      blockrequest_sched_ptr->add_requests (id, requests);
      return true;
    }

    bool ForwardSchedulerImpl::on_block (BlockRef block)
    {
      blockresponse_sched_ptr->add_response (block);
      return true;
    }

    int ForwardSchedulerImpl::get_ttl (NodeId id, Hash content_name, BlockIndex block_index)
    {
      return blockrequest_sched_ptr->get_ttl (id, content_name, block_index);
    }

    float ForwardSchedulerImpl::get_utility (NodeId id, Hash content_name, BlockIndex block_index)
    {
      return blockrequest_sched_ptr->get_utility (id, content_name, block_index);
    }

    std::size_t ForwardSchedulerImpl::freeze_blocks (std::vector<BlockRef> blocks)
    {
      return node.content->frozen(std::move(blocks), true);
    }

    std::size_t ForwardSchedulerImpl::unfreeze_blocks (std::vector<BlockRef> blocks)
    {
      return node.content->frozen(std::move(blocks), false);
    }

    bool ForwardSchedulerImpl::broadcast_block (Hash name, BlockIndex index)
    {
      return node.content->broadcast (BlockRef(name, index));
    }

    std::vector<Neighbor> ForwardSchedulerImpl::get_neighbors() const
    {
      return node.neighbors->get();
    }

    std::size_t ForwardSchedulerImpl::get_num_of_neighbor() const
    {
      return (node.neighbors->get()).size();
    }

    void ForwardSchedulerImpl::request_blocks (std::vector<BlockRequestArgs> requests)
    {
      node.content->request (requests);
    }

	std::size_t ForwardSchedulerImpl::fwd_interests()
	{
	  return node.interests->announce();
	}

	std::size_t ForwardSchedulerImpl::fwd_metas()
	{
	  return node.content->announce_metadata();
	}

    std::uint32_t ForwardSchedulerImpl::get_sched_interval() const
    {
      return ForwardScheduler::get_sched_interval();
    }

	std::size_t ForwardSchedulerImpl::get_max_ttl() const
	{
		return 20;
	}

	std::size_t ForwardSchedulerImpl::get_storage() const
	{
		return 256;
	}

	std::size_t ForwardSchedulerImpl::get_bandwidth() const
	{
		return 20;
	}

    void ForwardSchedulerImpl::sched() // which will be called regularly
    {
      // all the nums will be used later for piroritized broadcast
      auto start = std::chrono::system_clock::now();

      int num_of_requests = blockrequest_sched_ptr->sched();
      int num_of_blocks = blockresponse_sched_ptr->sched();
      int num_of_meta = meta_sched_ptr->sched();
      int num_of_interests = interest_sched_ptr->sched();

      auto end = std::chrono::system_clock::now();

      auto delay = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();

      log.i("| scheduling delay: %v", delay);

      //// async task
      asynctask (&ForwardSchedulerImpl::sched, this).do_in (
              std::chrono::milliseconds (ForwardScheduler::get_sched_interval()));

    }

    void ForwardSchedulerImpl::schedule_interest_fwd () { interest_sched_ptr->sched(); }

    void ForwardSchedulerImpl::schedule_metadata_fwd () { meta_sched_ptr->sched(); }

    void ForwardSchedulerImpl::schedule_blockrequest_fwd () {  blockrequest_sched_ptr->sched(); }

    void ForwardSchedulerImpl::schedule_blockresponse_fwd () {blockresponse_sched_ptr->sched(); }

    const Logger* ForwardSchedulerImpl::get_logger() const
    {
      return &log;
    }


	////// blockresponsescheduler.cpp

    void BlockResponseScheduler::add_response(BlockRef blockid)
    {
        block_arrived_buf.insert(blockid);
    }

    std::size_t BlockResponseScheduler::sched()
    {
      for (auto it=block_arrived_buf.begin();
              it != block_arrived_buf.end(); it++)
      {
        block_to_schedule.insert (*it);
      }
      block_arrived_buf.clear();


      std::size_t num_of_blocks = block_to_schedule.size();

      sched_ptr->get_logger()->d("scheduling %v blocks...", num_of_blocks);

      if (num_of_blocks == 0)
          return 0;

      std::size_t max_ttl = sched_ptr->get_max_ttl();
      std::size_t storage = sched_ptr->get_storage();
      std::size_t bandwidth = sched_ptr->get_bandwidth();
      std::size_t num_of_neighbor = sched_ptr->get_num_of_neighbor();

      std::vector<std::vector<int>> ttls (num_of_neighbor,
              std::vector<int>(num_of_blocks, -1));

      std::vector<std::vector<float>> utils (num_of_neighbor,
              std::vector<float>(num_of_blocks, 0));


      //map blockid to seq from 0...n

      std::size_t seq = 0;

      block_to_seq.clear();
      auto it = block_to_schedule.begin();
      while (it != block_to_schedule.end())
      {
        block_to_seq.insert(std::make_pair(*it, seq));

        std::vector<Neighbor> neighbors = sched_ptr->get_neighbors();
        for (std::size_t i=0; i<neighbors.size(); i++)
        {
          NodeId node_id = neighbors[i].id;
          utils[i][seq] = sched_ptr->get_utility (node_id,
                                                 it->hash,
                                                 it->index);
          ttls[i][seq] = sched_ptr->get_ttl (node_id,
                                             it->hash,
                                             it->index);
        }
        seq++;
        it++;
      }

      // t = 0...max_ttl+1
      std::vector<std::vector<std::size_t>> sched_result (num_of_blocks,
              std::vector<std::size_t>(max_ttl+2));


      double max_util = LPSolver::solve (max_ttl,
                       num_of_blocks,
                       storage,
                       bandwidth,
                       num_of_neighbor,
                       ttls,
                       utils,
                       sched_result
                       );

      sched_ptr->get_logger()->i ("| overall utility: %v", max_util);

	  std::vector<BlockRef> blocks_to_freeze;
	  std::vector<BlockRef> blocks_to_unfreeze;
	  std::vector<BlockRef> blocks_to_broadcast;

      for (std::size_t c=0; c<sched_result.size();c++)
      {

        BlockRef block_id = get_blockid (c);
        if (sched_result[c][0] == 1)
        {
          //// freeze cache
		  blocks_to_freeze.push_back (block_id);

          if (sched_result[c][1] == 0)
          {
			blocks_to_broadcast.push_back (block_id);
  		    block_to_schedule.erase (block_id);
          }
        }
        else
        {
          //// unfreeze cache
          //
          blocks_to_unfreeze.push_back (block_id);
        }
      }

//      sched_ptr->get_logger()->d("sending freeze/unfreeze commands...");
	  sched_ptr->freeze_blocks (blocks_to_freeze);
	  sched_ptr->unfreeze_blocks (blocks_to_unfreeze);

//      sched_ptr->get_logger()->d("done.");
	  for (std::size_t c=0; c!=blocks_to_broadcast.size(); c++)
	  {
		sched_ptr->broadcast_block (blocks_to_broadcast[c].hash,
                                    blocks_to_broadcast[c].index);
	  }

//      sched_ptr->get_logger()->d("sending %v broadcast command...", blocks_to_broadcast.size());
      return blocks_to_broadcast.size(); // update the num_of_blocks to broadcast
    }

    BlockRef BlockResponseScheduler::get_blockid (std::size_t seq)
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

    //// blockrequestscheduler.cpp

    void BlockRequestScheduler::add_requests (NodeId id, std::vector<BlockRequestArgs> requests)
    {
      // nodeID is neighbor's node, not origin node's ID.
      for (auto it = requests.begin(); it != requests.end(); ++it)
      {
        insert_request (id, *it);
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
        if (desc.expire_time >= current_time)
        {
  	      auto ttl = std::chrono::duration_cast<std::chrono::microseconds>
              (desc.expire_time - current_time);
          auto arg
              = BlockRequestArgs (BlockRef (desc.content_name,desc.block_index),
                              desc.utility,
                              ttl,
  							  desc.requester,
                              desc.origin_location,
                              false);

          request_to_fwd.push_back(arg);
        }
        request_queue.pop();
        max_num_of_requests--;
      }

      if (request_to_fwd.size() > 0)
        sched_ptr->request_blocks (request_to_fwd);

      return request_to_fwd.size();
    }

    void BlockRequestScheduler::insert_request (NodeId nodeID, BlockRequestArgs request)
    {
      auto requests_per_node = request_desc_table.find(nodeID);
      auto current_time = sma::chrono::system_clock::now();

  	// change from relative ttl to absolute ttl locally
      auto expire_time = current_time + request.ttl<std::chrono::microseconds>();

      BlockRequestDesc desc (request.block.hash,
                             request.block.index,
                             request.utility,
                             expire_time,
                             nodeID,
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

	//// interestscheduler.cpp

    std::size_t InterestScheduler::sched()
    {
  	  return sched_ptr->fwd_metas();
    }


	///// metascheduler.cpp
    std::size_t MetaScheduler::sched()
    {
  	  return sched_ptr->fwd_metas();
    }

}
