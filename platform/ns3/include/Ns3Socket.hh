#ifndef NS3_SOCKET_H_
#define NS3_SOCKET_H_

#include "ns3/socket.h"

#include "AbstractSocket.hh"

namespace sma
{

class Ns3Socket : public AbstractSocket {
public:
  Ns3Socket(int domain, int type, int protocol);
  virtual ~Ns3Socket();

  int bind(SocketAddress addr);
  size_t recv(void* const buf, size_t len, int flags);
  size_t send(const void* const buf, size_t len, int flags);

private:
  uint16_t              bindPort;
  ns3::Address          bindAddr;
  ns3::Ptr<ns3::Socket> socket;
};

}

#endif