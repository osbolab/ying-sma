#include "net/SocketAddress.hh"

#include <cassert>

#ifdef WIN32
#include "net/Winsock.hh"
#else
#include <netinet/in.h>   // struct sockaddr
#include <sys/Socket.h>   // AF_***
#endif


namespace sma
{

SocketAddress::SocketAddress(Address address, std::uint16_t port)
  : addr(address), port(port) {}

sockaddr SocketAddress::to_sockaddr() const
{
  // wow never do this
  typedef union {
    sockaddr    sa;
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
  saddr.sin.sin_port = port;
  memset(saddr.sin.sin_zero, 0, 8);
  saddr.sin.sin_addr.S_un.S_un_b.s_b1 = addr.data[0];
  saddr.sin.sin_addr.S_un.S_un_b.s_b2 = addr.data[1];
  saddr.sin.sin_addr.S_un.S_un_b.s_b3 = addr.data[2];
  saddr.sin.sin_addr.S_un.S_un_b.s_b4 = addr.data[3];
  return saddr.sa;
}

void SocketAddress::print(std::ostream& os) const
{
  os << addr << ":" << port;
}

std::ostream& operator <<(std::ostream& os, const SocketAddress& addr)
{
  addr.print(os);
  return os;
}

}