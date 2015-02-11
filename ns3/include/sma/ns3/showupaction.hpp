#pragma once

#include <sma/ns3/action.hpp>

#include <sma/ccn/ccnnode.hpp>
#include <chrono>

namespace sma
{
class Ns3NodeContainer;

struct ShowupAction : Action {

  ShowupAction (Ns3NodeContainer& context,
                std::chrono::milliseconds interest_freq,
                std::chrono::milliseconds content_freq,
                std::chrono::milliseconds content_freq)
      : Action (context)
      , post_interest_freq (interest_freq)
      , post_content_freq (content_freq)
      , download_content_freq (content_freq)
  {}


  virtual void operator()() override
  {
    CcnNode& node = *(context->node);
    node.log.d ("Action: join in the network");
    node.behavior->behave();
  }


  std::chrono::milliseconds post_interest_freq;
  std::chrono::milliseconds post_content_freq;
  std::chrono::milliseconds download_content_freq;

};
}
