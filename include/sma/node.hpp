#pragma once

#include <sma/actor.hpp>
#include <sma/neighbortable.hpp>

#include <chrono>

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

private:
  static constexpr auto INITIAL_BEACON_DELAY = std::chrono::milliseconds(1000);
  static constexpr auto BEACON_DELAY = std::chrono::milliseconds(5000);

  void schedule_beacon(std::chrono::milliseconds delay);
  void beacon();

  bool disposed{false};
};
}
