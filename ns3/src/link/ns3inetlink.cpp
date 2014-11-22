#include <sma/link/ns3inetlink.hpp>
#include <sma/link/linkmanager.hpp>

#include <ns3/ptr.h>
#include <ns3/node.h>
#include <ns3/socket.h>
#include <ns3/packet.h>
#include <ns3/inet-socket-address.h>

#include <cstdint>
#include <cassert>
#include <utiliity>


namespace sma
{
Ns3InetLink::Ns3InetLink(ns3::Ptr<ns3::Node> this_node)
{
  auto socket_factory_tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
  sock = ns3::Socket::CreateSocket(this_node, socket_factory_tid);
  assert(sock);

  bind();
}
Ns3InetLink::Ns3InetLink(Ns3InetLink&& r)
  : Link(std::move(r))
  , sock(r.sock)
{
  delete r.sock;
}
Ns3InetLink& Ns3InetLink::operator=(Ns3InetLink&& r)
{
  Link::operator=(std::move(r));
  sock = r.sock;
  delete r.sock;
  return *this;
}
Ns3InetLink::~Ns3InetLink()
{
  close();
}

void Ns3InetLink::bind()
{
  auto saddr = ns3::InetSocketAddress(ns3::Ipv4Address("0.0.0.0"), 9999);
  assert(sock->Bind(saddr) != -1);

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

std::size_t write(void const* src, std::size_t size)
{
  assert(sock);
  auto ip = ns3::Ipv4Address("10.1.1.255");
  auto saddr = ns3::InetSocketAddress(ip, 9999);
  std::size_t sent = sock->SendTo(src, size, 0, saddr);
  assert(sent == size);

  return sent;
}
std::size_t read(void* dst, std::size_t size)
{
  assert(sock);
  size = packet->CopyData(dst, size);
  readable(false);

  return size;
}
void close()
{
  if (sock)
    sock->Close();
}
}
