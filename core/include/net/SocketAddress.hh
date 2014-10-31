#pragma once

#include <cstdint>

#include "Address.hh"

// From <WinSock2.h> or <netinet/in.h>
struct sockaddr;

namespace sma
{

class SocketAddress final
{
public:
  SocketAddress(Address address, std::uint16_t port);

  virtual void print(std::ostream& os) const;
  friend std::ostream& operator <<(std::ostream& os, const SocketAddress& addr);

  sockaddr to_sockaddr() const;

  const Address         addr;
  const std::uint16_t   port;

};

}