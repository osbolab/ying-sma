#pragma once

#include <sma/ns3/action.hpp>

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <sma/ccn/ccnnode.hpp>
#include <sma/ccn/contenthelper.hpp>

#include <string>
#include <vector>

#include <istream>
#include <sstream>

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
    char data[62 * 1024];
    std::stringbuf sbuf;
    sbuf.pubsetbuf(data, sizeof data);
    std::istream is(&sbuf);

    CcnNode& node = *(context->node);
    node.log.d("Action: publish new content");
    node.content->publish(type, name, is);
  }

  ContentType type;
  ContentName name;
};
}
