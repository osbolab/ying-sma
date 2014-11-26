#pragma once

#include <sma/actor.hpp>
#include <sma/message.hpp>
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
  virtual ~Node() {}

  NodeId id() const { return id_; }

  // Actor

  virtual void receive(Message msg) override;

  inline void post(Message m);
  template <typename M>
  inline void post(M&& m);
  template <typename M, typename... Args>
  inline void post(M&& m, Args&&... args);

private:
  NodeId id_;

  NeighborHelper neighborHelper;
};

void Node::post(Message m) { Actor::post(std::move(m)); }

template <typename M>
void Node::post(M&& m)
{
  post(to_message(id(), std::forward<M>(m)));
}

template <typename M, typename... Args>
void Node::post(M&& m, Args&&... args)
{
  post(make_message<M>(id(), std::forward<Args>(args)...));
}
}
