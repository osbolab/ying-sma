#include <sma/linklayer.hpp>

#include <sma/messageheader.hpp>
#include <sma/messagetypes.hpp>

#include <sma/io/log>

#include <cstring>
#include <cassert>

namespace sma
{
LinkLayer::LinkLayer(std::vector<std::unique_ptr<Link>> links)
  : links(std::move(links))
  , send_os(&send_sbuf)
  , recv_is(&recv_sbuf)
  , writer(&send_os)
  , reader(&recv_is)
{
  send_sbuf.pubsetbuf(send_buf, sizeof send_buf);
  recv_sbuf.pubsetbuf(recv_buf, sizeof recv_buf);

  assert(!this->links.empty());
  for (auto& link : this->links)
    link->manager = this;
}

struct ReadAndDispatch {
  template <typename M, typename Reader>
  static void receive(Reader&& reader, MessageHeader header, Node* node)
  {
    if (!node)
      return;

    M message(std::forward<Reader>(reader));
    node->receive(std::move(header), std::move(message));
  }
};

void LinkLayer::on_link_readable(Link* link)
{
  if (!node)
    return;

  std::size_t read = 0;
  while ((read = link->read(recv_buf, sizeof recv_buf))) {
    // lock buffer

    MessageHeader header(reader);
    if (!MessageTypes::apply<ReadAndDispatch>(reader, std::move(header), node))
      LOG(WARNING) << "Unhandled message";

    recv_sbuf.pubseekpos(0);
    // unlock buffer
  }
}
}
