#pragma once

#include <sma/messagetypes.hpp>
#include <sma/messageheader.hpp>
#include <sma/messagebuffer.hpp>

#include <sma/util/binaryformat.hpp>
#include <sma/util/bufferdest.hpp>

namespace sma
{
class CcnNode;
class Link;
class ForwardStrategy;

class LinkLayer
{
public:
  virtual ~LinkLayer() {}

  //! Gracefully stop communication between the node and the links.
  virtual void stop() = 0;

  //! Set the node that will receive messages received by the link layer.
  void receive_to(CcnNode& node);

  //! Set the strategy used to manage the outgoing message queue.
  /*! If a forwarding strategy is set, it will be invoked after an outgoing
   * message is buffered and must call the link layer's \a forward_one
   * method to send the message.
   */
  LinkLayer& forward_strategy(ForwardStrategy& strategy);
  //! Forward the next queued outgoing message to the network.
  /*! \return the number of queued messages remaining.
   */
  virtual std::size_t forward_one() = 0;

  //! Place the message in the outgoing message queue.
  template <typename M>
  void enqueue(MessageHeader const& header, M const& msg);

protected:
  //! The target of incoming messages.
  CcnNode* node{nullptr};

  //! The strategy for forwarding outgoing messages to the network.
  /*! Naive blasting to contended networks will cause avoidable loss. The
   * strategy should make intelligent decisions to share the network capacity
   * with other peers.
   */
  ForwardStrategy* fwd_strat = nullptr;

private:
  /* A side effect of templatized message passing: we have to define the
   * enqueue function here, and the lock object needs to be in scope
   * in that function, so we have to totally ruin our decoupling.
   */
  //! Get the next outgoing message buffer for writing.
  virtual MessageBuffer::write_lock lock_message_buffer() = 0;

  friend class Link;
  //! Callback from link implementations when they have data to be read.
  virtual void on_link_readable(Link& link) = 0;

  //! Notify when an outgoing message is enqueued.
  virtual void on_message_enqueued() = 0;
};

template <typename M>
void LinkLayer::enqueue(MessageHeader const& header, M const& msg)
{
  {
    // The returned object is actually a scoped write lock on a shared buffer...
    // BUT YOU'D NEVER GUESS IT FROM THIS AWFUL CODE
    auto lock = lock_message_buffer();
    auto& buf = *lock;
    BufferDest dst(buf.data, sizeof buf.data);
    // clang-format off
    dst.format<BinaryOutput>()
    << header
    << MessageTypes::typecode<M>()
    << msg;
    // clang-format on
    buf.size = dst.size();
  }    // release the locked buffer

  on_message_enqueued();
}
}
