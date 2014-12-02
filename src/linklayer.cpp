#include <sma/linklayer.hpp>
#include <sma/ccn/ccnnode.hpp>

namespace sma
{
void LinkLayer::receive_to(CcnNode& node) { this->node = &node; }

LinkLayer& LinkLayer::forward_strategy(ForwardStrategy& strategy)
{
  this->fwd_strat = &strategy;
  return *this;
}
}
