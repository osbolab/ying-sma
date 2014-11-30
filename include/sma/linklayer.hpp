#pragma once

#include <sma/link.hpp>
#include <sma/sendstrategy.hpp>
#include <sma/broadcastrejectpolicy.hpp>

#include <sma/messagetypes.hpp>
#include <sma/messageheader.hpp>

#include <sma/util/ringbuffer.hpp>
#include <sma/util/reader.hpp>
#include <sma/util/binaryformat.hpp>

#include <mutex>
#include <vector>
#include <memory>
#include <cassert>

#include <istream>
#include <ostream>
#include <sstream>

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

    std::size_t size;
    std::stringbuf::char_type data[1024];
  };

public:
  LinkLayer(std::vector<std::unique_ptr<Link>> links);

  LinkLayer(LinkLayer const& r) = delete;
  LinkLayer& operator=(LinkLayer const& r) = delete;

  LinkLayer& send_strategy(SendStrategy& strat);
  LinkLayer& receive_to(CcnNode& node);
  void stop();

  template <typename M>
  void enqueue(MessageHeader const& header, M const& msg);

  std::size_t forward_one();

private:
  void on_link_readable(Link& link);

  std::mutex readmx;
  using Lock = std::lock_guard<std::mutex>;

  std::vector<std::unique_ptr<Link>> links;
  CcnNode* node{nullptr};

  SendStrategy* send_strat;
  // Sending and receiving are mutually thread-safe
  RingBuffer<MessageData, 16> send_buf;
  std::stringbuf::char_type recv_buf[1024];
  std::stringbuf recv_sbuf;
  // Don't reorder the below without changing the initializer.
  std::istream recv_is;
  Reader<BinaryInput> reader;

  BroadcastRejectPolicy rejects;
};


template <typename M>
void LinkLayer::enqueue(MessageHeader const& header, M const& msg)
{
  {
    auto slot = send_buf.claim();
    MessageData& buf = *slot;

    std::stringbuf sbuf;
    sbuf.pubsetbuf(buf.data, sizeof buf.data);
    std::ostream os(&sbuf);
    BinaryOutput writer(&os);

    writer << header << MessageTypes::typecode<M>() << msg;

    buf.size = os.tellp();
  }

  if (send_strat)
    send_strat->notify();
  else
    forward_one();
}
}
