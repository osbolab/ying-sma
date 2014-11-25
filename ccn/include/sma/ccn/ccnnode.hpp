#pragma once

#include <sma/node.hpp>

namespace sma
{
struct NodeId;
struct Message;
struct Context;

class CcnNode final : public Node
{
public:
  CcnNode(NodeId id, Context* ctx);

  virtual void receive(Message const& msg) override;
};
}
