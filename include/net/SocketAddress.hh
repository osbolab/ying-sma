#ifndef SOCKET_ADDRESS_H_
#define SOCKET_ADDRESS_H_

#include "Address.hh"


namespace sma
{

class SocketAddress {
public:
  SocketAddress(Address& address, std::uint16_t port)
    : addr(addr), port(port) {}

private:
  const Address&        addr;
  const std::uint16_t   port;
};

}

#endif