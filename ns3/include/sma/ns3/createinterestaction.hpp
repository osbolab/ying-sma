#pragma once

#include <sma/ns3/action.hpp>

#include <sma/ccn/interest.hpp>

#include <sma/ccn/ccnnode.hpp>
#include <sma/ccn/interesthelper.hpp>

#include <vector>

namespace sma
{
class Ns3NodeContainer;

struct CreateInterestAction : Action {
  using interest_vec = std::vector<Interest>;

  CreateInterestAction(Ns3NodeContainer& context, interest_vec interests)
    : Action(context)
    , interests(interests)
  {
  }

  virtual void operator()() override
  {
    CcnNode& node = *(context->node);
    node.log.d("Action: add %v interests", interests.size());
    node.interests->create_local(interests);
  }

  interest_vec interests;
};
}
