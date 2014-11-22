#include <sma/link/linkmanager.hpp>

#include <sma/message.hpp>
#include <sma/serialize.hpp>
#include <sma/binaryformat.hpp>

#include <istream>
#include <ostream>
#include <sstream>
#include <cstring>
#include <cassert>


namespace sma
{
LinkManager::LinkManager(std::vector<std::unique_ptr<Link>> links)
  : links(std::move(links))
  , send_os(&send_sbuf)
  , recv_os(&recv_sbuf)
  , serializer(BinaryFormatter<std::ostream>(&send_os))
  , deserializer(BinaryFormatter<std::istream>(&recv_os))
{
  assert(!this->links.empty());
  for (auto& link : this->links)
    link->manager = this;

  send_sbuf.pubsetbuf(send_buf, sizeof send_buf);
  recv_sbuf.pubsetbuf(recv_buf, sizeof recv_buf);
}
/*
LinkManager::LinkManager(LinkManager&& r)
  : links(std::move(r.links))
  , ibx(r.ibx)
{
  r.ibx = nullptr;
}
LinkManager& LinkManager::operator=(LinkManager&& r)
{
  std::swap(links, r.links);
  std::swap(ibx, r.ibx);
  return *this;
}
*/

void LinkManager::accept(Message const& msg)
{
  send_sbuf.pubseekpos(0);
  msg.write_fields(&serializer);
  std::size_t const msg_size = send_os.tellp();

  std::size_t wrote = 0;
  for (auto& link : links)
    assert((wrote = link->write(send_buf, msg_size)));
}

void LinkManager::on_link_readable(Link* link)
{
  if (!ibx)
    return;

  std::size_t read = 0;
  while ((read = link->read(recv_buf, sizeof recv_buf))) {
    ibx->accept(Message(&deserializer));
    recv_sbuf.pubseekpos(0);
  }
}
}
