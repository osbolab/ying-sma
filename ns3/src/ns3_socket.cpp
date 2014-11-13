#include <sma/ns3/ns3_socket.hpp>

#include <ns3/socket.h>
#include <ns3/address.h>
#include <ns3/node.h>
#include <ns3/ptr.h>

#include <cassert>
#include <cstdint>


namespace sma
{

static ns3::Address ns3_address(const socket_addr& addr)
{
  return ns3::InetSocketAddress(
      ns3::Ipv4Address(static_cast<std::uint32_t>(addr.addr)), addr.port);
}

/******************************************************************************
 * ns3_socket c/dtors
 */
ns3_socket::ns3_socket(ns3::TypeId socket_factory_tid, ns3::Ptr<ns3::Node> node)
{
  assert((sock = ns3::Socket::CreateSocket(node, socket_factory_tid)));
}

ns3_socket::ns3_socket(ns3_socket&& rhs)
  : sock(std::move(rhs.sock))
{
}
ns3_socket& ns3_socket::operator=(ns3_socket&& rhs)
{
  std::swap(sock, rhs.sock);
  return *this;
}
/* ns3_socket c/dtors
 *****************************************************************************/


/******************************************************************************
 * ns3_socket public member functions
 */
void ns3_socket::bind(const socket_addr& address)
{
  assert(sock);
  if (!sock->Bind(ns3_address(address)))
      throw_last_error();
}

void ns3_socket::close()
{
  if (sock && !sock->Close())
    throw_last_error();
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

/* ns3_socket public member functions
 *****************************************************************************/

void ns3_socket::throw_last_error()
{
  assert(sock);
  throw socket_exception(sock->GetErrno());
}
}
