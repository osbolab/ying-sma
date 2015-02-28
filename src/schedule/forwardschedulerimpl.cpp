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
#include <utility>
#include <unordered_set>
#include <algorithm>
#include <queue>

namespace sma
{

//    std::size_t ForwardSchedulerImpl::total_bandwidth =5 ; 
    // 20 is a good choice
    std::uint32_t ForwardSchedulerImpl::interval_per_packet = 20; 
    // 100 is a good choice
    std::size_t ForwardSchedulerImpl::sample_cycles = 100;

    using namespace std::placeholders;

    ForwardSchedulerImpl::ForwardSchedulerImpl(CcnNode& host_node, std::uint32_t interval)
        : ForwardScheduler (host_node, interval)
        , used_bandwidth (0)
        , cycles (0)
        , bytes_sent_block_o_fifo (0)
    {
      interest_sched_ptr = new InterestScheduler(this);
      meta_sched_ptr = new MetaScheduler(this);
      blockrequest_sched_ptr = new BlockRequestScheduler(this);
      blockresponse_sched_ptr = new BlockResponseScheduler(this);

      node.content->on_block_arrived() += std::bind(&ForwardSchedulerImpl::on_block, this, _1, _2);
      node.content->on_blocks_requested() += std::bind(&ForwardSchedulerImpl::on_blockrequest, this, _1, _2);
      node.content->on_request_timeout() += std::bind(&ForwardSchedulerImpl::on_block_timeout, this, _1);

      // add time_out async callback later

      asynctask (&ForwardSchedulerImpl::broadcast_block_fifo, this).do_in (
              std::chrono::milliseconds (interval_per_packet));
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

    void ForwardSchedulerImpl::broadcast_block_fifo()
    {
      if (block_o_fifo.size() > 0) {
        BlockRef block = block_o_fifo.front();
        broadcast_block (block.hash, block.index, bytes_sent_block_o_fifo);
//        broadcast_block (block.hash, block.index, bytes_sent_block_o_fifo);
        block_o_fifo.pop_front();
      }
      asynctask (&ForwardSchedulerImpl::broadcast_block_fifo, this).do_in (
              std::chrono::milliseconds (interval_per_packet));
    }

    bool ForwardSchedulerImpl::on_block_timeout (BlockRef block)
    {
      node.log.d ("block %v %v is timed out", block.hash, block.index);
      node.log.d ("");
      return true;
    }

    bool ForwardSchedulerImpl::on_blockrequest (NodeId id, std::vector<BlockRequestArgs> requests)
    {
      blockrequest_sched_ptr->add_requests (id, requests);
      return true;
    }

    bool ForwardSchedulerImpl::on_block (NodeId id, BlockRef block)
    {
      blockresponse_sched_ptr->add_response (id, block);
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

    std::size_t ForwardSchedulerImpl::freeze_blocks (std::unordered_set<BlockRef> blocks)
    {
      return node.content->frozen(std::vector<BlockRef>(blocks.begin(), blocks.end()), true);
    }

    std::size_t ForwardSchedulerImpl::unfreeze_blocks (std::unordered_set<BlockRef> blocks)
    {
      return node.content->frozen(std::vector<BlockRef>(blocks.begin(), blocks.end()), false);
    }

    bool ForwardSchedulerImpl::broadcast_block (Hash name, BlockIndex index, std::uint16_t & bytes_sent)
    {
      return node.content->broadcast (BlockRef(name, index), bytes_sent);
    }

    std::vector<Neighbor> ForwardSchedulerImpl::get_neighbors() const
    {
      return node.neighbors->get();
    }

    std::unordered_set<NodeId> ForwardSchedulerImpl::get_request_nodes() const
    {
      return blockrequest_sched_ptr->get_request_nodes(); 
    }

    NodeId ForwardSchedulerImpl::get_node_id() const
    {
      return node.id; 
    }

    std::size_t ForwardSchedulerImpl::get_num_of_neighbor() const
    {
      return (node.neighbors->get()).size();
    }

    std::uint16_t ForwardSchedulerImpl::request_blocks (std::vector<BlockRequestArgs> requests)
    {
      return node.content->request (requests);
    }

	std::uint16_t ForwardSchedulerImpl::fwd_interests()
	{
      node.log.d ("forwarding interests");
	  return node.interests->announce();
	}

	std::uint16_t ForwardSchedulerImpl::fwd_metas()
	{
      node.log.d ("forwarding meta data");
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
		return 64;
	}

	std::size_t ForwardSchedulerImpl::get_bandwidth() const
	{
		return next_interval/interval_per_packet - used_bandwidth;
	}

    void ForwardSchedulerImpl::reset_bandwidth()
    {
        used_bandwidth = 0; 
    }

    void ForwardSchedulerImpl::clear_request (Hash hash, BlockIndex index)
    {
      blockrequest_sched_ptr->clear_request (hash, index); 
    }

    void ForwardSchedulerImpl::delete_request_from_node (NodeId id, Hash hash, BlockIndex index)
    {
      blockrequest_sched_ptr->delete_request_from_node (id, hash, index); 
    }

    std::vector<BlockRequestDesc> ForwardSchedulerImpl::get_requests_for_block (BlockRef block) const
    {
      return blockrequest_sched_ptr->get_requests_for_block (block); 
    }
    
    double ForwardSchedulerImpl::get_transmission_range() const
    {
      return 1000; 
    }

    Vec2d ForwardSchedulerImpl::get_self_position() const
    {
      return node.position();  
    }

    Vec2d ForwardSchedulerImpl::get_node_position(NodeId id) const
    {
      return node.neighbors->get_position (id);
    }

    bool ForwardSchedulerImpl::has_request_for_block (BlockRef block) const
    {
      return blockrequest_sched_ptr->has_request_for_block (block);   
    }

    void ForwardSchedulerImpl::write_o_block (BlockRef block)
    {
      block_o_fifo.push_back (std::move(block));
    }

    void ForwardSchedulerImpl::sched() // which will be called regularly
    {
      auto start = std::chrono::system_clock::now();

      std::uint16_t bytes_sent = 0;
      std::uint16_t bytes_sent_on_request = 0; 
      std::uint16_t bytes_sent_on_block = 0;
      std::uint16_t bytes_sent_on_interest = 0;
      std::uint16_t bytes_sent_on_meta = 0;
          
      bytes_sent_on_block = bytes_sent_block_o_fifo;
      bytes_sent_block_o_fifo = 0; // reset the counter

      bytes_sent_on_request = schedule_blockrequest_fwd();
      
      // OTHERWISE: the bitrate is set too large.

      //// async task
      // based on which, calculate the bandwidth in this time sched interval.
      float ratio = (rand()%80 - 40) / 100.0;
      next_interval = ForwardScheduler::get_sched_interval() * (1 + ratio);
      asynctask (&ForwardSchedulerImpl::sched, this).do_in (
              std::chrono::milliseconds (next_interval));

      assert (block_o_fifo.empty());
      schedule_blockresponse_fwd();

      if (cycles % sample_cycles == 0) {
        bytes_sent_on_interest = schedule_interest_fwd();
        bytes_sent_on_meta = schedule_metadata_fwd();
      }
      cycles = (cycles+1) % sample_cycles;


      bytes_sent = bytes_sent_on_request + bytes_sent_on_block
          + bytes_sent_on_interest + bytes_sent_on_meta;

      auto end = std::chrono::system_clock::now();

      auto delay = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();

      log.i(" scheduling delay: %v", delay);
      log.i(" total bytes sent: %v", bytes_sent);
      log.i(" bytes sent on request: %v", bytes_sent_on_request);
      log.i(" bytes sent on block: %v", bytes_sent_on_block);
      log.i(" bytes sent on interest: %v", bytes_sent_on_interest);
      log.i(" bytes sent on meta: %v", bytes_sent_on_meta);
      log.i("");

    }

    std::uint16_t ForwardSchedulerImpl::schedule_interest_fwd () 
    { 
        return interest_sched_ptr->sched(); 
    }

    std::uint16_t ForwardSchedulerImpl::schedule_metadata_fwd () 
    { 
        return meta_sched_ptr->sched(); 
    }

    std::uint16_t ForwardSchedulerImpl::schedule_blockrequest_fwd () 
    { 
        return  blockrequest_sched_ptr->sched(); 
    }

    std::uint16_t ForwardSchedulerImpl::schedule_blockresponse_fwd () 
    { 
        return blockresponse_sched_ptr->sched(); 
    }

    const Logger* ForwardSchedulerImpl::get_logger() const
    {
      return &log;
    }


	////// blockresponsescheduler.cpp

    void BlockResponseScheduler::add_response(NodeId id, BlockRef blockid)
    {      
        //avoid resending the block to the requeste
        //Todo: delete_from_position of from_node?
        sched_ptr->get_logger()->d ("Block response from NodeId: %v", id);
        sched_ptr->delete_request_from_node (id, blockid.hash, blockid.index); 

        // fist hop in the respond chain
        // block is not from the network,but from the cache/store.
        if (id == sched_ptr->get_node_id()) {
          block_arrived_buf.insert (blockid);
          return;
        }

        std::vector<BlockRequestDesc> requests = 
            sched_ptr->get_requests_for_block (blockid);
        
        auto reqIt = requests.begin();
        while (reqIt != requests.end()) {
          Vec2d requester_location = sched_ptr->get_node_position(reqIt->from);
          Vec2d broadcaster_location = sched_ptr->get_node_position(id);

          double distance = Vec2d::distance (requester_location, broadcaster_location);
          sched_ptr->get_logger()->d ("distance is %v", distance);
          if ( distance >  sched_ptr->get_transmission_range()) {
            
            block_arrived_buf.insert(blockid);
            return;
          }
          else
          {
            sched_ptr->get_logger()->d ("block not scheduled: %v %v", blockid.hash, blockid.index);
            sched_ptr->clear_request(blockid.hash, blockid.index);
            reqIt++; 
          }
        }
    }

    std::uint16_t BlockResponseScheduler::sched()
    {
      for (auto it=block_arrived_buf.begin();
              it != block_arrived_buf.end(); it++)
      {
        // cancel broadcast if overhearing the block
        //Todo: also depends on the distance
//        if (block_to_schedule.find(*it) != block_to_schedule.end()) {
//          sched_ptr->clear_request(it->hash, it->index);
//          block_to_schedule.erase(*it);
//        }
//        else
          if (sched_ptr->has_request_for_block (*it))
            block_to_schedule.insert (*it);
      }
      block_arrived_buf.clear();


      std::size_t num_of_blocks = block_to_schedule.size();

      if (num_of_blocks == 0)
          return 0;

      std::size_t max_ttl = sched_ptr->get_max_ttl();
      std::size_t storage = sched_ptr->get_storage();
      std::size_t bandwidth = sched_ptr->get_bandwidth();
 //     std::size_t num_of_neighbor = sched_ptr->get_num_of_neighbor();
      auto nodeset = sched_ptr->get_request_nodes();
      std::size_t num_of_nodes = nodeset.size();

      std::vector<std::vector<int>> ttls (num_of_nodes,
              std::vector<int>(num_of_blocks, -1));

      std::vector<std::vector<float>> utils (num_of_nodes,
              std::vector<float>(num_of_blocks, 0));


      //map blockid to seq from 0...n

      std::size_t seq = 0;

      block_to_seq.clear();
      auto it = block_to_schedule.begin();
      while (it != block_to_schedule.end())
      {
        block_to_seq.insert(std::make_pair(*it, seq));

//        std::vector<Neighbor> neighbors = sched_ptr->get_neighbors();
//        for (std::size_t i=0; i<neighbors.size(); i++) 
        auto nodesetIt = nodeset.begin();
        std::size_t i=0;
        while (nodesetIt != nodeset.end())
        {
//          NodeId node_id = neighbors[i].id;
          NodeId node_id = *nodesetIt;
          utils[i][seq] = sched_ptr->get_utility (node_id,
                                                 it->hash,
                                                 it->index);

          ttls[i][seq] = sched_ptr->get_ttl (node_id,
                                             it->hash,
                                             it->index);
          nodesetIt++;
          i++;
        }
        seq++;
        it++;
      }

      // t = 0...max_ttl+1
      std::vector<std::vector<std::size_t>> sched_result (num_of_blocks,
              std::vector<std::size_t>(max_ttl+2));


      sched_ptr->get_logger()->d("%v blocks to schedule...", num_of_blocks);

      double max_util = LPSolver::solve (max_ttl,
                       num_of_blocks,
                       storage,
                       bandwidth,
                       num_of_nodes,
                       ttls,
                       utils,
                       sched_result
                       );

      sched_ptr->get_logger()->i ("| overall utility: %v", max_util);

	  std::unordered_set<BlockRef> blocks_to_freeze;
	  std::unordered_set<BlockRef> blocks_to_unfreeze;
	  std::unordered_set<BlockRef> blocks_to_broadcast;

      int bandwidth_reserved = 0;
      for (std::size_t c=0; c<sched_result.size();c++)
      {
        
        BlockRef block_id = get_blockid (c);
        if (sched_result[c][0] == 1)
        {
          //// freeze cache
		  blocks_to_freeze.insert (block_id);

          if (sched_result[c][1] == 0)
          {
			blocks_to_broadcast.insert (block_id);
  		    block_to_schedule.erase (block_id);
            bandwidth_reserved++;
          }
        }
        else
        {
         //// unfreeze cache
         blocks_to_unfreeze.insert (block_id);
         // should erase rejected blocks
         block_to_schedule.erase(block_id);
         if (sched_ptr->has_request_for_block(block_id))
           sched_ptr->get_logger()->d ("block %v %v is rejected", block_id.hash, block_id.index);
        }
      }

      auto freeze_block_it = blocks_to_freeze.begin();
      while (bandwidth_reserved < sched_ptr->get_bandwidth()
              && freeze_block_it != blocks_to_freeze.end())
      {
        blocks_to_broadcast.insert (*freeze_block_it);
        block_to_schedule.erase (*freeze_block_it);
        blocks_to_unfreeze.insert (*freeze_block_it);
        freeze_block_it = blocks_to_freeze.erase (freeze_block_it);
        bandwidth_reserved++;
//        freeze_block_it++;
      }


/*      auto unfrozen_block_it = blocks_to_unfreeze.begin();
      while (bandwidth_reserved < sched_ptr->get_bandwidth()
              && unfrozen_block_it != blocks_to_unfreeze.end())
      {
        blocks_to_broadcast.push_back (*unfrozen_block_it);
        block_to_schedule.erase (*unfrozen_block_it);
        bandwidth_reserved++;
        unfrozen_block_it++;
      }
*/
	  sched_ptr->freeze_blocks (blocks_to_freeze);
	  sched_ptr->unfreeze_blocks (blocks_to_unfreeze);


      std::uint16_t bytes_sent = 0;

      sched_ptr->get_logger()->d("sending %v broadcast command...", blocks_to_broadcast.size());
      for (auto br_block_it = blocks_to_broadcast.begin();
              br_block_it != blocks_to_broadcast.end(); br_block_it++)
	  {
        /// BUG: should broadcast each block twice.
//		sched_ptr->broadcast_block (br_block_it->hash, br_block_it->index, bytes_sent);
        sched_ptr->write_o_block (BlockRef(br_block_it->hash, br_block_it->index));
//        sched_ptr->write_o_block (BlockRef(br_block_it->hash, br_block_it->index));
//		sched_ptr->broadcast_block (br_block_it->hash, br_block_it->index, bytes_sent);
        sched_ptr->clear_request (br_block_it->hash, br_block_it->index);
	  }


//      return blocks_to_broadcast.size(); // update the num_of_blocks to broadcast
      return bytes_sent;
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
//      Vec2d self_position = sched_ptr->get_self_position();
      for (auto it = requests.begin(); it != requests.end(); ++it)
      {
        //CAUTION: requester_location is the origin location
//        Vec2d origin_position = it->requester_position; 
//        Vec2d requester_position = sched_ptr->get_node_position(id);
//        if (Vec2d::distance (self_position, origin_position)
//                < 2 * Vec2d::distance (requester_position, origin_position))
          insert_request (id, *it);
      }
      
//      asynctask (&BlockRequestScheduler::refresh_forwarded_requests, this).do_in (
//              std::chrono::milliseconds (50 * sched_ptr->get_sched_interval()));

    }

/*    void BlockRequestScheduler::refresh_forwarded_requests()
    {
      auto current_time = sma::chrono::system_clock::now();
      auto fwd_request_it = forwarded_requests.begin();
      while (fwd_request_it != forwarded_requests.end()) {
        if (fwd_request_it->second < current_time)
          fwd_request_it = forwarded_requests.erase(fwd_request_it);
        else
          fwd_request_it++;
      }
    }
*/

    std::vector<BlockRequestDesc> BlockRequestScheduler::get_requests_for_block (BlockRef block) const
    {
      std::vector<BlockRequestDesc> result;
      auto req_tb_it = request_desc_table.begin();
      while (req_tb_it != request_desc_table.end()) {
        auto req_it = (req_tb_it->second).begin();
        while (req_it != (req_tb_it->second).end()) {
          if (block.hash == req_it->content_name
                  && block.index == req_it->block_index)
            result.push_back (*req_it);
          req_it++;
        }
        req_tb_it++; 
      }

      return std::move(result);
    }


    void BlockRequestScheduler::clear_request (Hash hash, BlockIndex index)
    {
      auto reqIt = request_desc_table.begin();
      while (reqIt != request_desc_table.end())
      {
        auto descIt = (reqIt->second).begin();
        while (descIt != (reqIt->second).end())
        {
          if (descIt->content_name == hash
                  && descIt->block_index == index)
            (reqIt->second).erase (descIt);
          else
            descIt++;
        }
        if ( (reqIt->second).size() == 0 )
          reqIt = request_desc_table.erase (reqIt);
        else
          reqIt++; 
      }
    }

    void BlockRequestScheduler::delete_request_from_node (NodeId id, Hash hash, BlockIndex index)
    {
      auto reqIt = request_desc_table.find (id);
      if (reqIt != request_desc_table.end()) {
        // Todo: remove expired requests
        auto descIt = (reqIt->second).begin();
        while (descIt != (reqIt->second).end())
        {
          if (descIt->content_name == hash
                  && descIt->block_index == index)
            (reqIt->second).erase(descIt);
          else
            descIt++;
        }

        if ( (reqIt->second).size() == 0)
          request_desc_table.erase(reqIt);
      }
    }

	std::uint16_t BlockRequestScheduler::sched()
	{
	  return fwd_requests(std::max<std::size_t>(10,sched_ptr->get_bandwidth())); 
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
  			int time_span = std::chrono::duration_cast<std::chrono::milliseconds>(deadline
  				- current_time).count();
//              return (time_span / sched_ptr->get_sched_interval())/2-1;
              return time_span / sched_ptr->get_sched_interval()-1;
            }
            else
            {
              (requests_per_node->second).erase(it);
              sched_ptr->get_logger()->d ("clearing request for %v %v due to ttl",
                      it->content_name, it->block_index);
              if ( (requests_per_node->second).size() == 0)
                  request_desc_table.erase(requests_per_node);
              return -1;
            }
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
              sched_ptr->get_logger()->i (
                      "expired! current_time >  expire_time: %v",
                      std::chrono::duration_cast<std::chrono::milliseconds>(
                                       (current_time - requestIt->expire_time)).count());
              (requests_per_node->second).erase(requestIt);
              return 0;
            }

            target_utility = requestIt->utility;
          }
          total_utility += requestIt->utility;
          requestIt++;
        }
        return total_utility == 0 ? 0 :  (target_utility / total_utility);
      }
      else
      {
        return 0;
      }
    }

    bool BlockRequestScheduler::has_request_for_block(BlockRef block) const
    {
      auto req_tb_it = request_desc_table.begin();
      while (req_tb_it != request_desc_table.end())
      {
        auto req_it = (req_tb_it->second).begin();
        auto current_time = sma::chrono::system_clock::now();
        while (req_it != (req_tb_it->second).end())
        {
          if (req_it->content_name == block.hash
                  && req_it->block_index == block.index
                  && req_it->expire_time > current_time)
            return true;
          req_it++; 
        }
        req_tb_it++; 
      }
      return false;
    }

    std::uint16_t BlockRequestScheduler::fwd_requests (std::size_t max_num_of_requests)
    {
      std::vector<BlockRequestArgs> request_to_fwd;
      while (!request_queue.empty() && max_num_of_requests > 0)
      {
        BlockRequestDesc desc = request_queue.front();

        // translate from BlockRequestDesc to BlockRequestArgs, all about the ttl
        auto current_time = sma::chrono::system_clock::now();
        if (desc.expire_time >= current_time)
        {
  	      auto ttl = std::chrono::duration_cast<std::chrono::milliseconds>
              (desc.expire_time - current_time);
          BlockRef block (desc.content_name, desc.block_index);
          auto arg
              = BlockRequestArgs (block,
                              desc.utility,
                              ttl,
//  							  desc.requester,
                              sched_ptr->get_node_id(),
                              desc.origin_location,
                              desc.hops_from_origin+1,
                              false);

          request_to_fwd.push_back(arg);

//          forwarded_requests.insert (std::make_pair(block, 
//                      std::chrono::time_point_cast<Ms>(desc.expire_time)));
        }
        request_queue.pop();
        max_num_of_requests--;
      }

      std::uint16_t bytes_sent = 0;
      if (request_to_fwd.size() > 0)
        bytes_sent = sched_ptr->request_blocks (request_to_fwd);

//      return request_to_fwd.size();
      return bytes_sent;
    }

    void BlockRequestScheduler::insert_request (NodeId nodeID, BlockRequestArgs request)
    {

      auto requests_per_node = request_desc_table.find(nodeID);
      auto current_time = sma::chrono::system_clock::now();

  	// change from relative ttl to absolute ttl locally
      auto expire_time = current_time + request.ttl<std::chrono::milliseconds>();

      BlockRequestDesc desc (request.block.hash,
                             request.block.index,
                             request.utility,
                             expire_time,
                             nodeID,
                             request.requester,
                             request.requester_position,
                             request.hops_from_block);

      assert (nodeID == request.requester);

/*      // do not forward the request if it has been broadcast
      // but the request info should be still updated, e.g.,
      // ttl, utility
      auto fwd_request_it = forwarded_requests.find (BlockRef (desc.content_name, desc.block_index));
      if (fwd_request_it  == forwarded_requests.end()
              || fwd_request_it->second < current_time) {
      {
        request_queue.push(desc);
        if (fwd_request_it != forwarded_requests.end())
          forwarded_requests.erase (fwd_request_it);
      }
      else
        sched_ptr->get_logger()->d ("request discarded %v %v", desc.content_name, desc.block_index);
*/
      
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

    std::unordered_set<NodeId> BlockRequestScheduler::get_request_nodes () const
    {
        std::unordered_set<NodeId> nodeid_set;
        auto reqIt = request_desc_table.begin();
        while (reqIt != request_desc_table.end())
        {
          nodeid_set.insert (reqIt->first);
          reqIt++;
        }

        return std::move(nodeid_set);
    }   

	//// interestscheduler.cpp

    std::uint16_t InterestScheduler::sched()
    {
  	  return sched_ptr->fwd_interests();
    }


	///// metascheduler.cpp
    std::uint16_t MetaScheduler::sched()
    {
  	  return sched_ptr->fwd_metas();
    }

}
