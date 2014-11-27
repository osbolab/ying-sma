#pragma once

#include <sma/neighborhelper.hpp>
#include <sma/ccn/interesthelper.hpp>

namespace sma
{
struct Context;
struct NodeId;

struct MessageHeader;
struct NeighborMessage;
struct InterestMessage;

class Node
{
public:
  Node(NodeId id, Context* ctx);

  NodeId id() const { return id_; }

  void receive(MessageHeader header, NeighborMessage msg);
  void receive(MessageHeader header, InterestMessage msg);

  void add_interests(std::vector<ContentType> types);

private:
  NodeId id_;

  NeighborHelper neighborHelper;
  InterestHelper interestHelper;
};
}
