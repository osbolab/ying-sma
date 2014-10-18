#pragma once

#include "SocketFactory.hh"


namespace sma
{

class BsdSocketFactory : public SocketFactory
{
public:
  BsdSocketFactory();

  int create(Address::Family family, Socket::Type type, Socket::Protocol protocol,
             std::unique_ptr<Socket>& socket) override;
};

}