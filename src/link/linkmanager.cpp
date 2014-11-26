#include <sma/link/linkmanager.hpp>

#include <sma/message.hpp>

#include <sma/io/log>

#include <cstring>
#include <cassert>

namespace sma
{
LinkManager::LinkManager(std::vector<std::unique_ptr<Link>> links)
  : links(std::move(links))
  , send_os(&send_sbuf)
  , recv_is(&recv_sbuf)
  , serializer(&send_os)
  , deserializer(&recv_is)
{
  assert(!this->links.empty());
  for (auto& link : this->links)
    link->manager = this;

  send_sbuf.pubsetbuf(send_buf, sizeof send_buf);
  recv_sbuf.pubsetbuf(recv_buf, sizeof recv_buf);
}

void LinkManager::accept(Message&& msg)
{
  if (links.empty())
    return;
  // lock buffer
  send_sbuf.pubseekpos(0);
  serializer << msg;
  std::size_t const msg_size = send_os.tellp();

  std::size_t wrote = 0;
  for (auto& link : links)
    assert((wrote = link->write(send_buf, msg_size)));
  // unlock buffer
}

void LinkManager::on_link_readable(Link* link)
{
  if (!ibx)
    return;

  std::size_t read = 0;
  while ((read = link->read(recv_buf, sizeof recv_buf))) {
    // lock buffer
    Message msg(&deserializer);
    recv_sbuf.pubseekpos(0);
    // unlock buffer
    ibx->accept(std::move(msg));
  }
}
}
