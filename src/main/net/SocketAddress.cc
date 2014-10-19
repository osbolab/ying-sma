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

SocketAddress::SocketAddress(const Address& address, std::uint16_t port)
  : addr(address), port(port) {}

SocketAddress::SocketAddress(Address&& address, std::uint16_t port)
  : addr(std::move(address)), port(port) {}

SocketAddress::operator sockaddr() const
{
  sockaddr saddr;
  switch (addr.family) {
    case Address::IPv4:
      saddr.sa_family = AF_INET;
      break;
    default:
      assert(!1 && "Unsupported address family");
      break;
  }
  std::copy(addr.addr.begin(), addr.addr.begin() + 14, saddr.sa_data);
  return saddr;
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