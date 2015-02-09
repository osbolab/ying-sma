#pragma once

#include <sma/ccn/interest.hpp>
#include <sma/ccn/contentmetadata.hpp>
#include <utility>

namespace sma
{

class ForwardScheduler
{
public:
	
  ForwardScheduler (CcnNode* host_node, std::uint32_t interval)
	  : node (host_node)
	  , sched_interval (interval)
  {}
	  
  virtual void on_blockrequest (const std::vector<BlockRequestArgs> & requests) = 0;
  virtual void on_block (const std::vector<std::pair<Hash, std::size_t>> & blocks)= 0;
  virtual void sched() = 0;

  virtual ~ForwardScheduler() {}
  
  std::uint32_t get_sched_interval() const
  {
	return sched_interval;
  }
  
  CcnNode* get_node() const
  {
	return node;
  }
  
  
private:
	
  CcnNode* node;
  std::uint32_t sched_interval;
};
}
