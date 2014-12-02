#pragma once

#include <sma/link.hpp>
#include <sma/linklayer.hpp>

#include <sma/messagebuffer.hpp>

#include <sma/util/reader.hpp>
#include <sma/util/binaryformat.hpp>
#include <sma/util/buffersource.hpp>

#include <mutex>
#include <vector>
#include <memory>
#include <cassert>

namespace sma
{
class CcnNode;

class LinkLayerImpl : public LinkLayer
{
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

  ~LinkLayerImpl() {}

  std::size_t forward_one() override;

  void stop() override;

private:
  MessageBuffer::write_lock lock_message_buffer() override;

  void on_link_readable(Link& link) override;

  void on_message_enqueued() override;

  //! The broadcast links comprising the local node's network interface.
  std::vector<std::unique_ptr<Link>> links;

  //! Buffer outgoing messages to be sent by the forwarding strategy.
  MessageBuffer::type send_buf;

  // Incoming messages are still synchronous; this is just so we can reuse the
  // stringbuf and istream;
  char recv_buf[1024];
  //! Wraps the receive buffer in an input stream.
  BufferSource recv_bufsrc;
  //! Deserializes data from the receive buffer input stream.
  Reader<BinaryInput> recv_reader;
};
}
