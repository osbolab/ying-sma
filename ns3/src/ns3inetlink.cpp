#include <sma/link/ns3inetlink.hpp>
#include <sma/link/linkmanager.hpp>
#include <sma/chrono>
#include <sma/io/log>

#include <ns3/ptr.h>
#include <ns3/node.h>
#include <ns3/socket.h>
#include <ns3/packet.h>
#include <ns3/inet-socket-address.h>

#include <cstdint>
#include <cassert>
#include <utility>
#include <ostream>
#include <stdexcept>


namespace sma
{
std::ostream& operator<<(std::ostream& os, ns3::InetSocketAddress const& sa)
{
  sa.GetIpv4().Print(os);
  return os << ":" << sa.GetPort();
}

Ns3InetLink::Ns3InetLink(ns3::Ptr<ns3::Node> this_node)
{
  auto factory_tid = ns3::TypeId::LookupByName(NS3_FACTORY_TYPENAME);
  sock = ns3::Socket::CreateSocket(this_node, factory_tid);
  assert(sock);

  bind();
}
Ns3InetLink::Ns3InetLink(Ns3InetLink&& r)
  : Link(std::move(r))
  , sock(r.sock)
{
}
Ns3InetLink& Ns3InetLink::operator=(Ns3InetLink&& r)
{
  Link::operator=(std::move(r));
  sock = r.sock;
  return *this;
}
Ns3InetLink::~Ns3InetLink() { close(); }

void Ns3InetLink::bind()
{
  auto saddr = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), BCAST_PORT);
  if (sock->Bind(saddr) == -1) {
    LOG(FATAL) << "Failed to bind socket on " << saddr
               << " (errno = " << int(sock->GetErrno()) << ")";
    throw std::runtime_error("Exception binding inet socket");
  }

  sock->SetAllowBroadcast(true);
  assert(sock->GetAllowBroadcast());
  sock->SetRecvCallback(
      ns3::MakeCallback(&Ns3InetLink::packet_available, this));
}

void Ns3InetLink::packet_available(ns3::Ptr<ns3::Socket> s)
{
  ns3::Address sender;
  while ((packet = s->RecvFrom(sender)))
    readable(true);
}

std::size_t Ns3InetLink::write(void const* src, std::size_t size)
{
  assert(sock);
  auto ip = ns3::Ipv4Address(BCAST_ADDR);
  auto saddr = ns3::InetSocketAddress(ip, BCAST_PORT);
  std::size_t sent = sock->SendTo(
      reinterpret_cast<std::uint8_t const*>(src), size, 0, saddr);
  assert(sent == size);

  LOG(TRACE) << "sent " << sent << " bytes";
  return sent;
}
std::size_t Ns3InetLink::read(void* dst, std::size_t size)
{
  if (!readable())
    return 0;

  assert(sock);
  size = packet->CopyData(reinterpret_cast<std::uint8_t*>(dst), size);
  readable(false);

  LOG(TRACE) << "received " << size << " bytes";
  return size;
}
void Ns3InetLink::close()
{
  if (sock)
    sock->Close();
}
}
