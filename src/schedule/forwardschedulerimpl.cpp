#include <sma/schedule/forwardscheduler.hpp>
#include <sma/schedule/forwardschedulerimpl.hpp>
#include <sma/ccn/ccnnode.hpp>
#include <sma/ccn/blockindex.hpp>
#include <sma/schedule/interestscheduler.hpp>
#include <sma/schedule/metascheduler.hpp>
#include <sma/schedule/blockrequestscheduler.hpp>
#include <sma/schedule/blockresponsescheduler.hpp>
#include <sma/neighbor.hpp>
#include <cstddef>
#include <sma/ccn/blockrequestargs.hpp>
#include <vector>

namespace sma
{
    ForwardSchedulerImpl::ForwardSchedulerImpl(CcnNode* host_node, std::uint32_t interval)
        : ForwardScheduler (host_node, interval)
    {
	  node = ForwardScheduler::get_node();
      interest_sched_ptr = new InterestScheduler(); 
      meta_sched_ptr = new MetaScheduler();
      blockrequest_sched_ptr = new BlockRequestScheduler();
      blockresponse_sched_ptr = new BlockResponseScheduler();
    }
	
    ForwardSchedulerImpl::~ForwardSchedulerImpl()
    {
      if (interest_sched_ptr != NULL)  delete interest_sched_ptr;
      if (meta_sched_ptr != NULL)  delete meta_sched_ptr;
      if (blockrequest_sched_ptr != NULL)  delete blockrequest_sched_ptr;
      if (blockresponse_sched_ptr != NULL)  delete blockresponse_sched_ptr;
    }
	
    void ForwardSchedulerImpl::on_blockrequest (const std::vector<BlockRequestArgs> & requests)
    {
      blockrequest_sched_ptr->add_requests(requests); 
    }
	
    void ForwardSchedulerImpl::on_block (const std::vector<std::pair<Hash, BlockIndex>> & blocks)
    {
      blockresponse_sched_ptr->add_responses(blocks); 
    }
	
    std::size_t ForwardSchedulerImpl::freeze_blocks (td::vector<Hash, BlockIndex> blocks)
    {
      return node->content->freeze (blocks); 
    }
	
    std::size_t ForwardSchedulerImpl::unfreeze_blocks (td::vector<Hash, BlockIndex> blocks)
    {
      return node->content->unfreeze (blocks);  
    }
	
    bool ForwardSchedulerImpl::broadcast_block (Hash name, BlockIndex index)
    {
      return node->content->request (name, index); 
    }
	
    std::vector<Neighbor> ForwardSchedulerImpl::get_neighbors() const
    {
      return node->neighbors->get(); 
    }
	
    std::size_t ForwardSchedulerImpl::get_num_of_neighbor() const
    {
      return (node->neighbors->get()).size();
    }
	
    void ForwardSchedulerImpl::request_blocks (std::vector<BlockRequestArgs> requests)
    {
      node->content->request (requests); 
    }
	
	std::size_t ForwardSchedulerImpl::fwd_interests()
	{
	  return node->interest->announce();
	}
	
	std::size_t ForwardSchedulerImpl::fwd_metas()
	{
	  return node->content->publish_metadata();
	}
	
    std::uint32_t ForwardSchedulerImpl::get_sched_interval() const
    {
      return ForwardScheduler::get_sched_interval();
    }
	
	std::size_t ForwardSchedulerImpl::get_max_ttl() const
	{
		return 10;
	}
	
	std::size_t ForwardSchedulerImpl::get_storage() const
	{
		return 1000;
	}
	
	std::size_t ForwardSchedulerImpl::get_bandwidth() const
	{
		return 100;
	}
	
    void ForwardSchedulerImpl::sched() // which will be called regularly
    {
      // all the nums will be used later for piroritized broadcast
    
      int num_of_requests = blockrequest_sched_ptr->sched();
      int num_of_blocks = blockrespond_sched_ptr->sched();
      int num_of_meta = meta_sched_ptr->sched(10);
      int num_of_interests = interest_sched_ptr->sched(10);

      //// async task
      // put sched() itself to the chain
    }
}