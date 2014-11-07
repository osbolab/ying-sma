#include "socketaddress.hh"

#include <netinet/in.h>    // struct sockaddr
#include <sys/socket.h>    // AF_***
#include <cstring>         // memset
#include <cassert>

namespace sma
{
SocketAddress::SocketAddress(Address address, std::uint16_t port)
  : addr(address)
  , port(port)
{
}

sockaddr SocketAddress::to_sockaddr() const
{
  typedef union {
    sockaddr sa;
    sockaddr_in sin;
  } addr_t;

  addr_t saddr;
  switch (addr.family) {
    case Address::IPv4:
      saddr.sin.sin_family = AF_INET;
      break;
    default:
      assert(!1 && "Unsupported address family");
      break;
  }
  saddr.sin.sin_port = htons(port);
  std::memset(saddr.sin.sin_zero, 0, 8);
  // wow never do this
  saddr.sin.sin_addr.s_addr
      = *reinterpret_cast<const std::uint32_t*>(&addr.data[0]);
  ;
  return saddr.sa;
  // you can tell I wrote this after 2am on a Friday
}

void SocketAddress::print(std::ostream& os) const { os << addr << ":" << port; }

std::ostream& operator<<(std::ostream& os, const SocketAddress& addr)
{
  addr.print(os);
  return os;
}
}
