#ifndef BSDSOCKET_H_
#define BSDSOCKET_H_

#include "UdpSocket.hh"

namespace sma
{

class BsdUdpSocketImpl : public UdpSocket
{
public:
  int bind(const Address& address);
  int close();

  std::unique_ptr<const Packet> recvFrom(Address& sender);
  int send(std::unique_ptr<const Packet> packet, const Address& recipient);
};

}

#endif