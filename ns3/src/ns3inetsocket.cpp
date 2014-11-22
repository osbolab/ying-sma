#include <sma/ns3/ns3inetsocket.hpp>
#include <sma/io/log>

#include <ns3/ptr.h>
#include <ns3/node.h>
#include <ns3/socket.h>
#include <ns3/packet.h>
#include <ns3/inet-socket-address.h>

#include <cassert>
#include <cstdint>
#include <iostream>


namespace sma
{

static ns3::InetSocketAddress ns3_address(socket_addr const& addr)
{
  return ns3::InetSocketAddress(
      ns3::Ipv4Address(static_cast<std::uint32_t>(addr.addr)), addr.port);
}


/******************************************************************************
 * Ns3InetSocket c/dtors
 */
Ns3InetSocket::Ns3InetSocket(ns3::Ptr<ns3::Node> node)
  : bind_addr(ns3::InetSocketAddress(ns3::Ipv4Address(), 0))
{
  auto socket_factory_tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
  sock = ns3::Socket::CreateSocket(node, socket_factory_tid);
  assert(sock);
}

Ns3InetSocket::Ns3InetSocket(Ns3InetSocket&& rhs)
  : sock(std::move(rhs.sock))
  , bind_addr(std::move(rhs.bind_addr))
{
}
Ns3InetSocket& Ns3InetSocket::operator=(Ns3InetSocket&& rhs)
{
  std::swap(sock, rhs.sock);
  std::swap(bind_addr, rhs.bind_addr);
  return *this;
}
Ns3InetSocket::~Ns3InetSocket() { close(); }
/* Ns3InetSocket c/dtors
 *****************************************************************************/


/******************************************************************************
 * Ns3InetSocket public member functions
 */
void Ns3InetSocket::bind(socket_addr const& address)
{
  assert(sock);
  auto saddr = ns3_address(address);
  if (sock->Bind(saddr) == -1)
    throw_last_error();

  sock->SetAllowBroadcast(true);
  assert(sock->GetAllowBroadcast());
  bind_addr = saddr;
  sock->SetRecvCallback(
      ns3::MakeCallback(&Ns3InetSocket::on_packet_available, this));
}

void Ns3InetSocket::close()
{
  if (sock)
    sock->Close();
}

std::size_t Ns3InetSocket::recv(std::uint8_t* dst, std::size_t len)
{
  assert(sock);
  return sock->Recv(dst, len, 0);
}

void Ns3InetSocket::send(std::uint8_t const* src,
                           std::size_t len,
                           socket_addr const& dest)
{
  assert(sock);
  if (sock->SendTo(src, len, 0, ns3_address(dest)) != len)
    throw_last_error();
}

void Ns3InetSocket::broadcast(std::uint8_t const* src, std::size_t len)
{
  assert(sock);
  auto ip = ns3::Ipv4Address("10.1.1.255");
  auto saddr = ns3::InetSocketAddress(ip, std::uint16_t{9999});
  if ((sock->SendTo(src, len, 0, saddr)) != len)
    throw_last_error();
}

void Ns3InetSocket::on_packet_available(ns3::Ptr<ns3::Socket> s)
{
  ns3::Ptr<ns3::Packet> p;
  ns3::Address sender;

  while ((p = s->RecvFrom(sender))) {
  }
}

/* Ns3InetSocket public member functions
 *****************************************************************************/

void Ns3InetSocket::throw_last_error()
{
  assert(sock);
  LOG(FATAL) << "Uncaught socket exception no: " << sock->GetErrno();
  throw socket_exception(sock->GetErrno());
}
}
