#include <sma/net/ns3_inet_channel.hpp>
#include <sma/net/ns3_inet_socket.hpp>

#include <sma/channel.hpp>
#include <sma/messenger.hpp>
#include <sma/message.hpp>

#include <sma/unique_cast>
#include <sma/log>

#include <ns3/ptr.h>
#include <ns3/packet.h>

#include <vector>
#include <memory>
#include <utility>
#include <cassert>


namespace sma
{
ns3_inet_channel::ns3_inet_channel() {}
ns3_inet_channel::ns3_inet_channel(ns3_inet_channel&& rhs)
  : inet_channel(std::move(rhs))
  , socks(std::move(rhs.socks))
{
}
ns3_inet_channel& ns3_inet_channel::operator=(ns3_inet_channel&& rhs)
{
  channel::operator=(std::move(rhs));
  std::swap(socks, rhs.socks);
  return *this;
}

ns3_inet_channel::~ns3_inet_channel() { close(); }

void ns3_inet_channel::add(std::unique_ptr<inet_socket> sock)
{
  auto ns3_sock = dynamic_cast<ns3_inet_socket*>(sock.release());
  assert(ns3_sock);

  ns3_sock->inbox(this);
  socks.emplace_back(ns3_sock);
}

void ns3_inet_channel::accept(message const& m)
{
  if (socks.empty())
    return;

  std::uint8_t buf[SEND_BUFFER_SIZE];
  const std::size_t len = m.serialize_to(buf, sizeof(buf));

  for (auto& s : socks)
    s->broadcast(buf, len);
}

void ns3_inet_channel::close()
{
  ibx = nullptr;
  for (auto& s : socks)
    s->close();
  socks.clear();
}

void ns3_inet_channel::on_packet(ns3::Ptr<ns3::Packet> p)
{
  if (!ibx) {
    LOG(WARNING) << "Channel received a packet but has no inbox to deliver to";
    return;
  }

// ns3::Packet::PeekData is slated for removal in the next version;
// it's still nice to save the array copy though.
#ifdef NS3_PACKET_PEEKDATA_DEPRECATED
  std::uint8_t buf[RECV_BUFFER_SIZE];
  const std::size_t len = p->CopyData(buf, sizeof(buf));
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

  auto buf = p->PeekData();
  const std::size_t len = p->GetSize();

#pragma GCC diagnostic pop
#endif
  // IN THIS THREAD
  // Pass this message all the way up the chain to whomever ends up handling
  // it in the application level.
  ibx->accept(message(buf, len));
  // THEN return and let the message contents go out of scope.
  // If this runs concurrently or is reentered, you might die.
}
}
