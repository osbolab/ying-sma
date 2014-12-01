#pragma once

#include <sma/nodeid.hpp>
#include <sma/context.hpp>
#include <sma/linklayer.hpp>

#include <sma/messageheader.hpp>

#include <sma/neighbors.hpp>
#include <sma/ccn/interests.hpp>
#include <sma/ccn/content.hpp>

#include <sma/io/log>

#include <vector>

namespace sma
{
class NeighborHelper;
class InterestHelper;
class ContentHelper;

struct NeighborMessage;
struct InterestMessage;
struct ContentInfoMessage;

class CcnNode
{
public:
  CcnNode(NodeId id, Context& context);
  ~CcnNode();

  CcnNode(CcnNode const&) = delete;
  CcnNode& operator=(CcnNode const&) = delete;

  void stop() { stopped = true; }

  template <typename M>
  void post(M const& msg);
  template <typename M>
  void post(std::vector<NodeId> recipients, M const& msg);

  void receive(MessageHeader&& header, NeighborMessage&& msg);
  void receive(MessageHeader&& header, InterestMessage&& msg);
  void receive(MessageHeader&& header, ContentInfoMessage&& msg);

  Neighbors& neighbors();
  Interests& interests();
  Content& content();

  NodeId const id;
  Context* const context;
  Logger log;

private:
  bool stopped = false;
  LinkLayer* linklayer;

  NeighborHelper* neighbor_helper;
  InterestHelper* interest_helper;
  ContentHelper* content_helper;
};

template <typename M>
void CcnNode::post(M const& msg)
{
  linklayer->enqueue(MessageHeader(id), msg);
}

template <typename M>
void CcnNode::post(std::vector<NodeId> recipients, M const& msg)
{
  linklayer->enqueue(MessageHeader(id, std::move(recipients)), msg);
}
}
