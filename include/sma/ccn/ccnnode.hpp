#pragma once

#include <sma/nodeid.hpp>

#include <sma/messagebuffer.hpp>
#include <sma/messageheader.hpp>

#include <sma/ccn/ccnfwd.hpp>

#include <sma/io/log>

#include <vector>
#include <atomic>

namespace sma
{
class LinkLayer;

class CcnNode
{
public:
  CcnNode(NodeId id, Context& context);

  CcnNode(CcnNode const&) = delete;
  CcnNode& operator=(CcnNode const&) = delete;

  //! Gracefully terminate further network delegation or asynchronous tasks.
  void stop() { stopped = true; }

  //! Enqueue the given message data to be broadcast to the network.
  void post(void const* src, std::size_t size);
  //! Serialize the given message and enqueue it to be broadcast.
  template <typename M>
  void post(M const& msg,
            std::vector<NodeId> recipients = std::vector<NodeId>());

  /* Neighbor discovery */
  void receive(MessageHeader header, Beacon msg);
  void receive(MessageHeader header, BeaconResponse msg);

  /* Interest dissemination */
  void receive(MessageHeader header, InterestAnn msg);

  /* Content announcement dissemination */
  void receive(MessageHeader header, ContentAnn msg);

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
  std::atomic_bool stopped{false};
  //! Interface to the link layer connecting this node to the network.
  LinkLayer* linklayer;
};

template <typename M>
void CcnNode::post(M const& msg, std::vector<NodeId> recipients)
{
  MessageBuffer<20000> buf(MessageHeader(id, std::move(recipients)), msg);
  post(buf.cdata(), buf.size());
}
}
