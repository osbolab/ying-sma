#pragma once

#include "Socket.hh"


namespace sma
{

class SocketFactory
{
public:
  virtual ~SocketFactory() {};

  virtual int create(Address::Family family, Socket::Type type, Socket::Protocol protocol,
                     std::unique_ptr<Socket>& socket) = 0;

protected:
  SocketFactory();
};

}