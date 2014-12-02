#pragma once

#include <sma/ns3/action.hpp>

#include <sma/ccn/contenttype.hpp>

#include <sma/ccn/ccnnode.hpp>
#include <sma/ccn/interesthelper.hpp>

#include <vector>

namespace sma
{
class Ns3NodeContainer;

struct CreateInterestAction : Action {
  using interests_type = std::vector<ContentType>;

  CreateInterestAction(Ns3NodeContainer& context, interests_type interests)
    : Action(context)
    , interests(interests)
  {
  }

  virtual void operator()() override
  {
    CcnNode& node = *(context->node);
    node.log.d("Action: add %v interests", interests.size());
    node.interests->insert_new(interests);
  }

  interests_type interests;
};
}
