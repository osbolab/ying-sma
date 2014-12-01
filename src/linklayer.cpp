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
  static Unmarshal instance;

  template <typename M, typename Reader>
  void operator()(MessageHeader&& header, Reader& reader, CcnNode& node)
  {
    node.receive(header, M(reader));
  }
};
// Avoid constructing it for every message
Unmarshal Unmarshal::instance;

LinkLayer::LinkLayer(std::vector<std::unique_ptr<Link>> links,
                     BroadcastRejectPolicy rejectif)
  : links(std::move(links))
  , rejectif(rejectif)
  , recv_bufsrc(recv_buf, sizeof recv_buf)
  , recv_reader(recv_bufsrc.reader<BinaryInput>())
{
  assert(!this->links.empty());
  for (auto& link : this->links)
    link->receive_to(*this);
}

LinkLayer& LinkLayer::receive_to(CcnNode& node)
{
  this->node = &node;
  return *this;
}

LinkLayer& LinkLayer::forward_strategy(ForwardStrategy& strategy)
{
  fwd_strat = &strategy;
  return *this;
}

void LinkLayer::stop() { node = nullptr; }

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
  while (node && link.read(recv_buf, sizeof recv_buf)) {
    recv_bufsrc.rewind();

    MessageHeader header(recv_reader);
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

    auto msg_typecode = recv_reader.template get<MessageTypes::typecode_type>();
    // Deduce the type of the message from its typecode and call the unmarshal
    // operator to deserialize an instance of that type and pass it to the node.
    assert(node);
    if (!MessageTypes::apply(msg_typecode,
                             Unmarshal::instance,
                             std::move(header),
                             recv_reader,
                             *node))
      node->log.w("Unhandled message");
  }
}
}
