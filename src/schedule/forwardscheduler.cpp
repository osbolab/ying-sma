#include <sma/schedule/forwardscheduler.hpp>
#include <sma/ccn/ccnnode.hpp>

namespace sma
{

    ForwardScheduler::ForwardScheduler(CcnNode& node, std::uint32_t interval)
        : Helper (node)
        , sched_interval (interval)
    {
      node.sched = this; 
    }
}
