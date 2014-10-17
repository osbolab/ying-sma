#pragma once

#include <memory>

#include "SocketAddress.hh"
#include "Packet.hh"


namespace sma
{

class Socket
{
public:
  enum Type { Datagram, };
  enum Protocol { Udp, };

  virtual ~Socket() {}

  virtual int bind(const SocketAddress& address) = 0;
  virtual void close() = 0;

  virtual std::unique_ptr<Packet> recv() = 0;
  virtual int send(std::unique_ptr<const Packet> packet, const SocketAddress& recipient) = 0;

  virtual int getLastError() const = 0;

protected:
  Socket() {}

  virtual int setLastError(int error) = 0;

  Address::Family family;
  Type            type;
  Protocol        protocol;
};

}