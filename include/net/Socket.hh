#ifndef ABSTRACTSOCKET_H_
#define ABSTRACTSOCKET_H_

#include <memory>

#include "Address.hh"
#include "Packet.hh"


namespace sma
{

class Socket
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