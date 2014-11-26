#pragma once

#include <sma/node.hpp>
#include <sma/ccn/interesthelper.hpp>

namespace sma
{
struct NodeId;
struct Message;
struct Context;

class CcnNode final : public Node
{
public:
  CcnNode(NodeId id, Context* ctx);

  virtual void receive(Message&& msg) override;

  void make_interest(std::vector<ContentType> types);

private:
  InterestHelper interestHelper;
};
}
