#pragma once

#include <sma/nodeid.hpp>
#include <sma/linklayer.hpp>
#include <sma/messageheader.hpp>
#include <sma/ccn/ccnfwd.hpp>

#include <sma/io/log>

#include <vector>

namespace sma
{
class CcnNode
{
public:
  CcnNode(NodeId id, Context& context);

  CcnNode(CcnNode const&) = delete;
  CcnNode& operator=(CcnNode const&) = delete;

  //! Gracefully terminate further network delegation or asynchronous tasks.
  void stop() { stopped = true; }

  //! Enqueue the given message to be broadcast to the network neighbors.
  template <typename M>
  void post(M const& msg);
  //! Enqueue the given message to be broadcast to only the named neighbors.
  template <typename M>
  void post(std::vector<NodeId> recipients, M const& msg);

  /* Messages delegated to their respective handlers. */

  void receive(MessageHeader&& header, Beacon&& msg);
  void receive(MessageHeader&& header, InterestAnnouncement&& msg);
  void receive(MessageHeader&& header, ContentAnnouncement&& msg);

  //! This node's universally unique identifier.
  NodeId const id;
  //! The application context in which this node is running.
  Context* const context;
  //! A logger tailored to provide context-sensitive output for this node.
  Logger log;

  NeighborHelper* neighbors = nullptr;
  InterestHelper* interests = nullptr;
  ContentHelper* content = nullptr;

private:
  //! \a true if the node's graceful termination has been requested.
  bool stopped = false;
  //! Interface to the link layer connecting this node to the network.
  LinkLayer* linklayer;
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
