#pragma once

#include <sma/ccn/interest.hpp>
#include <sma/ccn/contentmetadata.hpp>
#include <utility>
#include <sma/ccn/ccnnode.hpp>
#include <sma/ccn/blockrequestargs.hpp>
#include <sma/helper.hpp>

namespace sma
{

class ForwardScheduler : public Helper
{
public:
	
  ForwardScheduler (CcnNode& node, std::uint32_t interval);
	  
  virtual bool on_blockrequest (NodeId id, std::vector<BlockRequestArgs> requests) = 0;
  virtual bool on_block (BlockRef block)= 0;
  virtual void sched() = 0;

  virtual ~ForwardScheduler() {}
  
  std::uint32_t get_sched_interval() const
  {
	return sched_interval;
  }
  
private:
	
  std::uint32_t sched_interval;
};
}
