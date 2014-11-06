#pragma once

#include <cstdint>

#include "address.hh"

// From <WinSock2.h> or <netinet/in.h>
struct sockaddr;

namespace sma
{

class SocketAddress final
{
public:
  SocketAddress(Address address, uint16_t port);

  virtual void print(std::ostream& os) const;
  friend std::ostream& operator <<(std::ostream& os, const SocketAddress& addr);

  sockaddr to_sockaddr() const;

  const Address         addr;
  const uint16_t   port;

};

}