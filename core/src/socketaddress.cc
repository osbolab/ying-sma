#include "socketaddress.hh"

#ifdef WIN32
#include "winsock.hh"
#else
#include <netinet/in.h>    // struct sockaddr
#include <sys/socket.h>    // AF_***
#include <cstring>         // memset
#endif

#include <cassert>

namespace sma
{
SocketAddress::SocketAddress(Address address, uint16_t port)
  : addr(address)
  , port(port)
{
}

sockaddr
SocketAddress::to_sockaddr() const
{
  // FIXME: wow never do this
  typedef union {
    sockaddr sa;
    sockaddr_in sin;
  } addr_t;

  addr_t saddr;
  switch (addr.family) {
    case Address::IPv4: saddr.sin.sin_family = AF_INET; break;
    default: assert(!1 && "Unsupported address family"); break;
  }
  saddr.sin.sin_port = htons(port);
  std::memset(saddr.sin.sin_zero, 0, 8);
// FIXME: This is nice and safe...
#ifdef WIN32
  saddr.sin.sin_addr.S_un.S_addr =
      *reinterpret_cast<const uint32_t *>(&addr.data[0]);
#else
  saddr.sin.sin_addr.s_addr =
      *reinterpret_cast<const uint32_t *>(&addr.data[0]);
#endif
  ;
  return saddr.sa;
  // you can tell I wrote this after 2am on a Friday
}

void
SocketAddress::print(std::ostream &os) const
{
  os << addr << ":" << port;
}

std::ostream &operator<<(std::ostream &os, const SocketAddress &addr)
{
  addr.print(os);
  return os;
}
}
