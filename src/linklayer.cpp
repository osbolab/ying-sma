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

LinkLayer::LinkLayer(std::vector<std::unique_ptr<Link>> links)
  : links(std::move(links))
  , recv_is(&recv_sbuf)
  , reader(&recv_is)
{
  recv_sbuf.pubsetbuf(recv_buf, sizeof recv_buf);

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

LinkLayer& LinkLayer::send_strategy(SendStrategy& strategy)
{
  send_strat = &strategy;
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
    recv_sbuf.pubseekpos(0);

    MessageHeader header(reader);
    if (rejects.loopback && header.sender == node->id)
      continue;

    if (rejects.not_addressed_to_me && !header.recipients.empty()) {
      bool discard = true;
      for (auto& id : header.recipients)
        if (id == node->id) {
          discard = false;
          break;
        }
      if (discard)
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
