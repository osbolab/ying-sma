#include <sma/ns3/ns3_socket.hpp>
#include <sma/log.hpp>

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

static ns3::InetSocketAddress ns3_address(const socket_addr& addr)
{
  return ns3::InetSocketAddress(
      ns3::Ipv4Address(static_cast<std::uint32_t>(addr.addr)), addr.port);
}


/******************************************************************************
 * ns3_socket c/dtors
 */
ns3_socket::ns3_socket(ns3::TypeId socket_factory_tid, ns3::Ptr<ns3::Node> node)
  : bind_addr(ns3::InetSocketAddress(ns3::Ipv4Address(), 0))
{
  sock = ns3::Socket::CreateSocket(node, socket_factory_tid);
  assert(sock);
}

ns3_socket::ns3_socket(ns3_socket&& rhs)
  : sock(std::move(rhs.sock))
  , bind_addr(std::move(rhs.bind_addr))
{
}
ns3_socket& ns3_socket::operator=(ns3_socket&& rhs)
{
  std::swap(sock, rhs.sock);
  std::swap(bind_addr, rhs.bind_addr);
  return *this;
}
ns3_socket::~ns3_socket()
{
  close();
}
/* ns3_socket c/dtors
 *****************************************************************************/


/******************************************************************************
 * ns3_socket public member functions
 */
void ns3_socket::bind(const socket_addr& address)
{
  assert(sock);
  auto saddr = ns3_address(address);
  if (sock->Bind(saddr) == -1)
    throw_last_error();

  sock->SetAllowBroadcast(true);
  assert(sock->GetAllowBroadcast());
  bind_addr = saddr;
  sock->SetRecvCallback(ns3::MakeCallback(&ns3_socket::on_packet, this));
}

void ns3_socket::close()
{
  if (sock)
    sock->Close();
}

std::size_t ns3_socket::recv(std::uint8_t* dst, std::size_t len)
{
  assert(sock);
  return sock->Recv(dst, len, 0);
}

void ns3_socket::send(const std::uint8_t* src,
                      std::size_t len,
                      const socket_addr& dest)
{
  assert(sock);
  if (sock->SendTo(src, len, 0, ns3_address(dest)) != len)
    throw_last_error();
}

void ns3_socket::broadcast(const std::uint8_t* src, std::size_t len) {
  assert(sock);
  auto ip = ns3::Ipv4Address("10.1.1.255");
  auto saddr = ns3::InetSocketAddress(ip, std::uint16_t{9999});
  if ((sock->SendTo(src, len, 0, saddr)) != len)
    throw_last_error();
}

void ns3_socket::on_packet(ns3::Ptr<ns3::Socket> s)
{
  ns3::Ptr<ns3::Packet> p;
  ns3::Address sender;

  while ((p = s->RecvFrom(sender))) {
    inbound->on_packet(std::move(p));
  }
}

void ns3_socket::receive_to(ns3_channel* inbound) { this->inbound = inbound; }

/* ns3_socket public member functions
 *****************************************************************************/

void ns3_socket::throw_last_error()
{
  assert(sock);
  LOG(FATAL) << "Uncaught socket exception no: " << sock->GetErrno();
  throw socket_exception(sock->GetErrno());
}
}
