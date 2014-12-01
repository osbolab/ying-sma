#pragma once

#include <sma/link.hpp>
#include <sma/forwardstrategy.hpp>
#include <sma/broadcastrejectpolicy.hpp>

#include <sma/messagetypes.hpp>
#include <sma/messageheader.hpp>

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

class LinkLayer final
{
  friend class Link;

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
  LinkLayer(std::vector<std::unique_ptr<Link>> links,
            BroadcastRejectPolicy rejectif = BroadcastRejectPolicy());

  LinkLayer(LinkLayer const& r) = delete;
  LinkLayer& operator=(LinkLayer const& r) = delete;

  LinkLayer& forward_strategy(ForwardStrategy& strategy);
  LinkLayer& receive_to(CcnNode& node);
  void stop();

  template <typename M>
  void enqueue(MessageHeader const& header, M const& msg);

  std::size_t forward_one();

private:
  void on_link_readable(Link& link);

  std::vector<std::unique_ptr<Link>> links;
  CcnNode* node{nullptr};

  ForwardStrategy* fwd_strat;
  // Sending and receiving are mutually thread-safe
  RingBuffer<MessageData, 16> send_buf;

  // Outgoing messages are not yet buffered. This is just so we can reuse the
  // stringbuf and istream, but messages are sent upstream as soon as they
  // arrive.
  char recv_buf[1024];
  // For reading out of the receive buffer
  BufferSource recv_bufsrc;
  Reader<BinaryInput> recv_reader;

  BroadcastRejectPolicy const rejectif;
};


template <typename M>
void LinkLayer::enqueue(MessageHeader const& header, M const& msg)
{
  {
    auto& buf = *(send_buf.claim());
    BufferDest dbuf(buf.data, sizeof buf.data);

    // clang-format off
    dbuf.writer<BinaryOutput>()
      << header
      << MessageTypes::typecode<M>()
      << msg;
    // clang-format on

    buf.size = dbuf.size();
  }

  if (fwd_strat)
    fwd_strat->notify();
  else
    forward_one();
}
}
