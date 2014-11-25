#pragma once

#include <sma/actor.hpp>
#include <sma/neighbortable.hpp>

namespace sma
{
struct NodeId;
class Context;

class Node : public Actor
{
public:
  Node(NodeId id, Context* ctx);
  virtual ~Node();

  NodeId id() const { return id_; }

  // Actor

  virtual void dispose() override;
  virtual void receive(Message const& msg) override;

protected:
  NodeId id_;
  NeighborTable neighbors;
};
}
