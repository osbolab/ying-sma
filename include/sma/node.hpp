#pragma once

#include <sma/actor.hpp>
#include <sma/neighbortable.hpp>
#include <sma/neighbormessage.hpp>

#include <chrono>
#include <random>

namespace sma
{
class Context;
struct NodeId;


class Node : public Actor
{
public:
  Node(NodeId id, Context* ctx);
  virtual ~Node();

  NodeId id() const { return id_; }

  // Actor

  virtual void receive(Message const& msg) override;

protected:
  virtual NeighborMessage::body_type beacon_data();

  NodeId id_;
  NeighborTable neighbors;

private:
  static constexpr auto INITIAL_BEACON_DELAY = std::chrono::milliseconds(1000);
  std::chrono::milliseconds beacon_delay{5000};
  std::chrono::milliseconds prune_interval{1000};

  void schedule_beacon(std::chrono::milliseconds delay);
  void beacon();

  void prune_neighbors();

  std::default_random_engine rand;
  bool disposed{false};
};
}
