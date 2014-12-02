#pragma once

#include <sma/link.hpp>
#include <sma/linklayer.hpp>

#include <sma/util/ringbuffer.hpp>

#include <sma/util/reader.hpp>
#include <sma/util/binaryformat.hpp>
#include <sma/util/buffersource.hpp>

#include <mutex>
#include <vector>
#include <memory>
#include <cassert>

namespace sma
{
namespace detail
{
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

    char data[8192];
    std::size_t size;
  };
}

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

  void stop() override;

  std::size_t forward_one() override;

  void enqueue(void const* src, std::size_t size) override;

private:
  void on_link_readable(Link& link) override;

  //! The broadcast links comprising the local node's network interface.
  std::vector<std::unique_ptr<Link>> links;

  //! Buffer outgoing messages to be sent by the forwarding strategy.
  RingBuffer<detail::MessageData> send_buf;

  // Incoming messages are still synchronous; this is just so we can reuse the
  // stringbuf and istream;
  char recv_buf[1024];
  //! Wraps the receive buffer in an input stream.
  BufferSource recv_bufsrc;
  //! Deserializes data from the receive buffer input stream.
  Reader<BinaryInput> recv_reader;
};
}
