#pragma once

#include <sma/ns3/action.hpp>

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>
#include <sma/ccn/contentmetadata.hpp>

#include <sma/ccn/ccnnode.hpp>
#include <sma/ccn/contenthelper.hpp>

#include <string>
#include <vector>


namespace sma
{
class Ns3NodeContainer;

struct PublishContentAction : Action {
  using interests_type = std::vector<ContentType>;

  PublishContentAction(Ns3NodeContainer& context,
                       std::string type,
                       std::string name)
    : Action(context)
    , type(type)
    , name(name)
  {
  }

  virtual void operator()() override
  {
    char data[4 * 1024];
    std::memset(data, 'a', sizeof(data));

    CcnNode& node = *(context->node);
    node.log.d("Action: publish new content");
    auto metadata = node.content->create_new({type}, name, data, sizeof(data));
    node.content->announce_metadata();
  }

  ContentType type;
  ContentName name;
};
}
