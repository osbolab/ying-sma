#pragma once

#include <sma/link.hpp>
#include <sma/forwardstrategy.hpp>
#include <sma/broadcastrejectpolicy.hpp>

#include <sma/util/ringbuffer.hpp>

#include <sma/util/reader.hpp>
#include <sma/util/binaryformat.hpp>
#include <sma/util/buffersource.hpp>
#include <sma/util/bufferdest.hpp>

#include <mutex>
#include <vector>
#include <memory>
#include <cassert>

namespace sma
{
class CcnNode;

class LinkLayerImpl
{
  friend class Link;

  //! Represents one buffered serialized message.
  /*! These are meant to be modified in-place by claiming one from the buffer
   * and populating it.
   */
  struct MessageData {
    MessageData() = default;
    MessageData(MessageData&&) = delete;
    MessageData(MessageData const&) = delete;
    MessageData& operator=(MessageData&&) = delete;
    MessageData& operator=(MessageData const&) = delete;

    char data[1024];
    std::size_t size;
  };

public:
  //! Construct a link layer composed of the given links.
  /*! The link layer owns its links and they should not be mutated by any other
   * object during its lifetime.
   */
  LinkLayerImpl(std::vector<std::unique_ptr<Link>> links);

  //! Copying is disabled because the underlying links are non-copyable.
  LinkLayerImpl(LinkLayerImpl const& r) = delete;
  //! Copying is disabled because the underlying links are non-copyable.
  LinkLayerImpl& operator=(LinkLayerImpl const& r) = delete;

  //! Set the strategy used to manage the outgoing message queue.
  /*! If a forwarding strategy is set, it will be invoked after an outgoing
   * message is buffered and must call the link layer's \a forward_one
   * method to send the message.
   */
  LinkLayerImpl& forward_strategy(std::unique_ptr<ForwardStrategy> strategy);
  //! Deques the next message in the buffer and forwards it to the network.
  /*! \return the number of messages remaining in the buffer minus the one sent.
   */
  std::size_t forward_one();

  //! Gracefully stop communication between the node and the links.
  void stop();

private:
  //! Callback from child link implementations when they have data to be read.
  void on_link_readable(Link& link);

  //! The broadcast links comprising the local node's network interface.
  std::vector<std::unique_ptr<Link>> links;
  //! The strategy for forwarding outgoing messages to the network.
  /*! Naive blasting to contended networks may cause avoidable loss. The
   * strategy should make intelligent decisions to share the network capacity
   * with other instances.
   */
  std::unique_ptr<ForwardStrategy> fwd_strat;

  //! Buffer outgoing messages to be sent by the forwarding strategy.
  RingBuffer<MessageData, 16> send_buf;

  // Incoming messages are still synchronous; this is just so we can reuse the
  // stringbuf and istream;
  char recv_buf[1024];
  //! Wraps the receive buffer in an input stream.
  BufferSource recv_bufsrc;
  //! Deserializes data from the receive buffer input stream.
  Reader<BinaryInput> recv_reader;
};
}
