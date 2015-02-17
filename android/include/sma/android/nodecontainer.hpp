#pragma once

#include <sma/linklayerimpl.hpp>
//#include <sma/prforwardstrategy.hpp>
#include <sma/context.hpp>
#include <sma/component.hpp>

#include <sma/neighborhelperimpl.hpp>
#include <sma/ccn/interesthelperimpl.hpp>
#include <sma/ccn/contenthelperimpl.hpp>
#include <sma/ccn/behaviorhelperimpl.hpp>

#include <sma/schedule/forwardschedulerimpl.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <memory>

namespace sma
{
class NodeContainer
{
public:
  NodeContainer();
  ~NodeContainer();

private:
  std::unique_ptr<CcnNode> node;

  std::unique_ptr<Context> ctx;
  std::unique_ptr<LinkLayerImpl> linklayer;

  std::unique_ptr<NeighborHelperImpl> neighbor_helper;
  std::unique_ptr<InterestHelperImpl> interest_helper;
  std::unique_ptr<BehaviorHelperImpl> behavior_helper;
  std::unique_ptr<ForwardSchedulerImpl> scheduler_helper;

  std::unique_ptr<ContentHelperImpl> content_helper;

  std::vector<std::unique_ptr<Component>> components;
};
}
