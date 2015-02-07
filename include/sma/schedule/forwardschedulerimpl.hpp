#pragma once

#include <sma/schedule/forwardscheduler.hpp>
#include <sma/schedule/interestscheduler.hpp>
#include <sma/schedule/metascheduler.hpp>
#include <sma/schedule/blockrequestscheduler.hpp>
#include <sma/schedule/blockresponsescheduler.hpp>

namespace sma
{
class ForwardSchedulerImpl : public ForwardScheduler
{
public:
  ForwardSchedulerImpl()
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


private:
  InterestScheduler* interest_sched_ptr;
  MetaScheduler* meta_sched_ptr;
  BlockRequestScheduler* blockrequest_sched_ptr;
  BlockRespondScheduler* blockresponse_sched_ptr;

  void schedule_interest_fwd () { interest_sched_ptr->sched(); }
  void schedule_metadata_fwd () { meta_sched_ptr->sched(); }
  void schedule_blockrequest_fwd { blockrequest_sched_ptr->sched(); }
  void schedule_blockresponse_fwd () {blockresponse_sched_ptr->sched(); }
};
}
