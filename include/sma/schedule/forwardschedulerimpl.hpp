#pragma once

#include <sma/schedule/forwardscheduler.hpp>
#include <sma/schedule/interestscheduler.hpp>
#include <sma/schedule/metascheduler.hpp>
#include <sma/schedule/blockrequestscheduler.hpp>
#include <sma/schedule/blockresponsescheduler.hpp>
#include <sma/ccn/ccnnode.hpp>
#include <ctime>
#include <cstdlib>
#include <sma/neighbor.hpp>
#include <sma/ccn/blockrequestargs.hpp>
#include <cstddef>
#include <vector>
#include <utility>
#include <sma/ccn/blockref.hpp>
#include <sma/io/log>

namespace sma
{

class ForwardSchedulerImpl : public ForwardScheduler
{
public:
  ForwardSchedulerImpl(CcnNode& host_node, std::uint32_t interval);
  ~ForwardSchedulerImpl();
  
  bool on_blockrequest (NodeId id, std::vector<BlockRequestArgs>  requests) override;
  bool on_block (BlockRef block) override;
  void sched () override;

  std::size_t freeze_blocks (std::vector<BlockRef> blocks);
  std::size_t unfreeze_blocks (std::vector<BlockRef> blocks);
  bool broadcast_block (Hash name, BlockIndex index); 
  void request_blocks (std::vector<BlockRequestArgs> requests);
  std::size_t fwd_interests ();
  std::size_t fwd_metas ();

  
  std::vector<Neighbor> get_neighbors() const;
  std::size_t get_num_of_neighbor() const;
  std::uint32_t get_sched_interval() const;
  std::size_t get_max_ttl() const;
  std::size_t get_storage() const;
  std::size_t get_bandwidth() const;
  int get_ttl (NodeId id, Hash content_name, BlockIndex block_index);                                                                     
  float get_utility (NodeId id, Hash content_name, BlockIndex block_index); 
  const Logger* get_logger() const;


private:
  InterestScheduler* interest_sched_ptr;
  MetaScheduler* meta_sched_ptr;
  BlockRequestScheduler* blockrequest_sched_ptr;
  BlockResponseScheduler* blockresponse_sched_ptr;

  void schedule_interest_fwd ();
  void schedule_metadata_fwd ();
  void schedule_blockrequest_fwd ();
  void schedule_blockresponse_fwd ();
};
}
