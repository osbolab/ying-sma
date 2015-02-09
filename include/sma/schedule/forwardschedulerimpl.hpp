#pragma once

#include <sma/schedule/forwardscheduler.hpp>
#include <sma/schedule/interestscheduler.hpp>
#include <sma/schedule/metascheduler.hpp>
#include <sma/schedule/blockrequestscheduler.hpp>
#include <sma/schedule/blockresponsescheduler.hpp>
#include <sma/ccn/ccnnode.hpp>
#include <ctime>
#include <cstdlib>
#include <sma/ccn/blockindex.hpp>
#include <sma/neighbor.hpp>
#include <sma/ccn/blockrequestargs.hpp>
#include <cstddef>

namespace sma
{
class ForwardSchedulerImpl : public ForwardScheduler
{
public:
  ForwardSchedulerImpl(CcnNode* host_node, std::uint32_t interval);
  ~ForwardSchedulerImpl();
  
  void on_blockrequest (const std::vector<BlockRequestArgs> & requests) override;
  void on_block (const std::vector<std::pair<Hash, BlockIndex>> & blocks) override;
  void sched () override;

  void freeze_block (Hash name, BlockIndex index);
  void unfreeze_block (Hash name, BlockIndex index);
  void broadcast_block (Hash name, BlockIndex index); 
  void request_blocks (std::vector<BlockRequestArgs> requests);

  
  std::vector<Neighbor> get_neighbors() const;
  std::size_t get_num_of_neighbor() const;
  std::uint32_t get_sched_interval() const;
  std::size_t get_max_ttl() const;
  std::size_t get_storage() const;
  std::size_t get_bandwidth() const;



private:
  InterestScheduler* interest_sched_ptr;
  MetaScheduler* meta_sched_ptr;
  BlockRequestScheduler* blockrequest_sched_ptr;
  BlockRespondScheduler* blockresponse_sched_ptr;
  CcnNode* node;

  void schedule_interest_fwd () { interest_sched_ptr->sched(); }
  void schedule_metadata_fwd () { meta_sched_ptr->sched(); }
  void schedule_blockrequest_fwd () { blockrequest_sched_ptr->sched(); }
  void schedule_blockresponse_fwd () {blockresponse_sched_ptr->sched(); }
};
}
