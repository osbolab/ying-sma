#include <sma/ns3/ns3_channel.hpp>
#include <sma/ns3/ns3_socket.hpp>
#include <sma/channel.hpp>
#include <sma/messenger.hpp>
#include <sma/message.hpp>
#include <sma/socket.hpp>
#include <sma/log>

#include <ns3/ptr.h>
#include <ns3/packet.h>

#include <vector>
#include <utility>
#include <cassert>


namespace sma
{
ns3_channel::ns3_channel(ns3_socket* sock)
  : socks(1)
{
  socks[0] = std::move(sock);
  sock->receive_to(this);
}
ns3_channel::ns3_channel(std::vector<ns3_socket*> socks)
  : socks(std::move(socks))
{
  assert(this->inbox_);
  for (auto& sock : socks)
    sock->receive_to(this);
}

ns3_channel::ns3_channel(ns3_channel&& rhs)
  : channel(std::move(rhs))
  , socks(std::move(rhs.socks))
{
}
ns3_channel& ns3_channel::operator=(ns3_channel&& rhs)
{
  channel::operator=(std::move(rhs));
  std::swap(socks, rhs.socks);
  return *this;
}
ns3_channel::ns3_channel(const ns3_channel& rhs)
  : channel(rhs)
  , socks(rhs.socks)
{
}
ns3_channel& ns3_channel::operator=(const ns3_channel& rhs)
{
  channel::operator=(rhs);
  socks = rhs.socks;
  return *this;
}

ns3_channel::~ns3_channel() {}

void ns3_channel::accept(const message& m)
{
  if (socks.empty())
    return;

  std::uint8_t buf[SEND_BUFFER_SIZE];
  const std::size_t len = m.serialize_to(buf, sizeof(buf));

  for (auto& s : socks)
    s->broadcast(buf, len);
}

void ns3_channel::close()
{
  inbox_ = nullptr;
  for (auto& s : socks)
    s->close();
  socks.clear();
}

void ns3_channel::on_packet(ns3::Ptr<ns3::Packet> p)
{
  if (!inbox_) {
    LOG(WARNING) << "Channel has no sink to deliver to";
    return;
  }

// ns3::Packet::PeekData is slated for removal in the next version
// sorry about this, but hey, it saves us a pointless array copy
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
  inbox_->accept(message(buf, len));
  // THEN return and let the message contents go out of scope.
  // If this runs concurrently or is reentered, you might die.
}
}
