#pragma once

#include <sma/schedule/forwardscheduler.hpp>
#include <sma/schedule/interestscheduler.hpp>
#include <sma/schedule/metascheduler.hpp>
#include <sma/schedule/blockrequestscheduler.hpp>
#include <sma/schedule/blockresponsescheduler.hpp>
#include <sma/ccn/ccnnode.hpp>
#include <ctime>
#include <cstdlib>

namespace sma
{
class ForwardSchedulerImpl : public ForwardScheduler
{
public:
  ForwardSchedulerImpl(CcnNode* host_node, std::uint32_t interval)
      : node (host_node)
      , sched_interval (interval);
  {
    interest_sched_ptr = new InterestScheduler(); 
    meta_sched_ptr = new MetaScheduler();
    blockrequest_sched_ptr = new BlockRequestScheduler();
    blockresponse_sched_ptr = new BlockResponseScheduler();
  }

  ~ForwardSchedulerImpl()
  {
    if (interest_sched_ptr != NULL)  delete interest_sched_ptr;
    if (meta_sched_ptr != NULL)  delete meta_sched_ptr;
    if (blockrequest_sched_ptr != NULL)  delete blockrequest_sched_ptr;
    if (blockresponse_sched_ptr != NULL)  delete blockresponse_sched_ptr;
  }
  
  void on_blockrequest (const std::vector<BlockRequest>& requests)
  {
    blockrequest_sched_ptr->add_requests(requests); 
  }

  void on_blockresponse (const std::vector<BlockResponse*>& responses)
  {
    blockresponse_sched_ptr->add_responses(responses); 
  }

  void freeze_block (Hash name, size_t index)
  {
    content->freeze_block (name, index); 
  }

  void unfreeze_block (Hash name, size_t index)
  {
    content->unfreeze_block (name, index);  
  }

  void broadcast_block (Hash name, size_t index)
  {
    content->request_block (name, index); 
  }

  std::vector<Neighbor> get_neighbors()
  {
    return node->neighbors->get(); 
  }

  int get_num_of_neighbor() const
  {
    return (node->neighbors->get()).size();
  }
  void request_blocks (std::vector<BlockRequestArgs> requests)
  {
    node->content->request_blocks (requests); 
  }

  std::uint32_t get_sched_interval() const
  {
    return sched_interval; 
  }

  void sched ()  // which will be called regularly
  {
    // all the nums will be used later for piroritized broadcast
    
    int num_of_requests = blockrequest_sched_ptr->sched();
    int num_of_blocks = blockrespond_sched_ptr->sched();
    int num_of_meta = meta_sched_ptr->sched(10);
    int num_of_interests = interest_sched_ptr->sched(10);

    //// async task
    // put sched() itself to the chain
  }


private:
  InterestScheduler* interest_sched_ptr;
  MetaScheduler* meta_sched_ptr;
  BlockRequestScheduler* blockrequest_sched_ptr;
  BlockRespondScheduler* blockresponse_sched_ptr;
  CcnNode* node;
  std::uint32_t sched_interval;

  void schedule_interest_fwd () { interest_sched_ptr->sched(); }
  void schedule_metadata_fwd () { meta_sched_ptr->sched(); }
  void schedule_blockrequest_fwd { blockrequest_sched_ptr->sched(); }
  void schedule_blockresponse_fwd () {blockresponse_sched_ptr->sched(); }
};
}
