#include <sma/ccn/behaviorhelper.hpp>
#include <sma/ccn/ccnnode.hpp>

namespace sma
{

BehaviorHelper::BehaviorHelper (CcnNode& node)
    : Helper (node)
{
  node.behavior = this;
}

}
