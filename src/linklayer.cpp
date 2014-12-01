#include <sma/linklayer.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/messageheader.hpp>
#include <sma/messagetypes.hpp>

#include <sma/io/log>

#include <cstring>
#include <cassert>

#include <typeinfo>

namespace sma
{
struct Unmarshal {
  template <typename M, typename Reader>
  void operator()(MessageHeader&& header, Reader& reader, CcnNode& node)
  {
    node.receive(std::move(header), M(reader));
  }
};

LinkLayer::LinkLayer(std::vector<std::unique_ptr<Link>> links,
                     BroadcastRejectPolicy rejectif)
  : links(std::move(links))
  , rejectif(rejectif)
  , recv_sbuf(recv_buf, sizeof recv_buf)
  , reader(recv_sbuf.reader<BinaryInput>())
{
  assert(!this->links.empty());
  for (auto& link : this->links)
    link->receive_to(*this);
}

LinkLayer& LinkLayer::receive_to(CcnNode& node)
{
  Lock lock(readmx);
  this->node = &node;
  return *this;
}

LinkLayer& LinkLayer::forward_strategy(ForwardStrategy& strategy)
{
  fwd_strat = &strategy;
  return *this;
}

void LinkLayer::stop()
{
  // Don't take the node away while reading
  Lock lock(readmx);
  node = nullptr;
}

std::size_t LinkLayer::forward_one()
{
  {
    auto maybe_data = send_buf.try_pop();
    if (!maybe_data.first)
      return 0;

    auto const& cbuf = *(maybe_data.second);

    for (auto& link : links)
      assert(link->write(cbuf.data, cbuf.size));
  }

  return send_buf.size();
}

void LinkLayer::on_link_readable(Link& link)
{
  // Only take the lock once in the event that the read buffer keeps giving us
  // data. Theoretically if the network interface's read buffer is full forever
  // we'll deadlock, but a lot more than this needs to change when we're really
  // threading it.
  Lock readlock(readmx);

  std::size_t read = 0;
  while (node && (read = link.read(recv_buf, sizeof recv_buf))) {
    recv_sbuf.rewind();

    MessageHeader header(reader);
    // Reject broadcast loopbacks
    if (header.sender == node->id)
      continue;

    if (rejectif.not_addressed_to_me && !header.recipients.empty()) {
      auto const& this_node = node->id;
      auto const nrecp = header.recipients.size();
      decltype(header.recipients)::size_type i = 0;
      while (i++ < nrecp)
        if (header.recipients[i] == this_node)
          break;
      if (i == nrecp)
        continue;
    }

    auto typecode = reader.template get<MessageTypes::typecode_type>();
    // Deduce the type of the message from its typecode and call the unmarshal
    // operator to deserialize an instance of that type and pass it to the node.
    if (!MessageTypes::apply(
            typecode, Unmarshal(), std::move(header), reader, *node))
      node->log.w("Unhandled message");
  }
}
}
