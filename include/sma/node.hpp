#pragma once

#include <sma/actor.hpp>
#include <sma/neighborhelper.hpp>

#include <chrono>

namespace sma
{
class Context;
struct NodeId;
struct Message;

class Node : public Actor
{
public:
  Node(NodeId id, Context* ctx);
  virtual ~Node();

  NodeId id() const { return id_; }

  // Actor

  virtual void receive(Message&&  msg) override;

private:
  NodeId id_;
};
}
