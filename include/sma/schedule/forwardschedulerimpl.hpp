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
#include <unordered_set>
#include <sma/schedule/blockrequestdesc.hpp>
#include <list>
#include <sma/ccn/blockresponse.hpp>

namespace sma
{

class ForwardSchedulerImpl : public ForwardScheduler
{
public:
  ForwardSchedulerImpl(CcnNode& host_node, std::uint32_t interval);
  ~ForwardSchedulerImpl();
  
  bool on_blockrequest (NodeId id, std::vector<BlockRequestArgs>  requests) override;
  bool on_block (NodeId id, BlockRef block) override;
  bool on_block_timeout (BlockRef block) override;
  void sched () override;

  std::size_t freeze_blocks (std::unordered_set<BlockRef> blocks);
  std::size_t unfreeze_blocks (std::unordered_set<BlockRef> blocks);
  std::uint16_t request_blocks (std::vector<BlockRequestArgs> requests);
  std::uint16_t fwd_interests ();
  std::uint16_t fwd_metas ();

  
  std::vector<Neighbor> get_neighbors() const;
  std::size_t get_num_of_neighbor() const;
  std::uint32_t get_sched_interval() const;
  std::size_t get_max_ttl() const;
  std::size_t get_storage() const;
  std::size_t get_bandwidth() const;
  int get_ttl (NodeId id, Hash content_name, BlockIndex block_index);                                                                     
  float get_utility (NodeId id, Hash content_name, BlockIndex block_index); 
  const Logger* get_logger() const;
  NodeId get_node_id() const;
  void clear_request (Hash hash, BlockIndex index);
  void delete_request_from_node (NodeId id, Hash hash, BlockIndex index);
  std::unordered_set<NodeId> get_request_nodes () const;
  std::vector<BlockRequestDesc> get_requests_for_block (BlockRef block) const;
  Vec2d get_self_position() const;
  Vec2d get_node_position(NodeId id) const;
  bool has_request_for_block(BlockRef block) const;
  double get_transmission_range() const;
  void write_o_block(BlockRef block);
  void broadcast_block_fifo();
  bool broadcast_block (Hash name, BlockIndex index, std::uint16_t & bytes_sent); 

private:
  std::list<BlockRef> block_o_fifo;
  std::uint16_t bytes_sent_block_o_fifo;

  InterestScheduler* interest_sched_ptr;
  MetaScheduler* meta_sched_ptr;
  BlockRequestScheduler* blockrequest_sched_ptr;
  BlockResponseScheduler* blockresponse_sched_ptr;

//  static std::size_t total_bandwidth;
  std::size_t used_bandwidth;
  static std::size_t sample_cycles;
  std::size_t cycles;
  static std::uint32_t interval_per_packet;
  std::uint32_t next_interval;



  std::uint16_t schedule_interest_fwd ();
  std::uint16_t schedule_metadata_fwd ();
  std::uint16_t schedule_blockrequest_fwd ();
  std::uint16_t schedule_blockresponse_fwd ();

  void reset_bandwidth();
};
}
