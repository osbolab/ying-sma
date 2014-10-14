#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_

#include "Socket.hh"

namespace sma
{

class UdpSocket : public Socket
{
public:
  virtual int bind(const Address& address) = 0;
  virtual int close() = 0;

  virtual std::unique_ptr<const Packet> recvFrom(Address& sender) = 0;
  virtual int send(std::unique_ptr<const Packet> packet,
                   const Address& recipient) = 0;

};

}

#endif