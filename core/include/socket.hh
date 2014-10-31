#pragma once

#include <memory>

#include "socketaddress.hh"
#include "message.hh"


namespace sma
{

class Socket
{
public:
  enum Type { Datagram, };
  enum Protocol { Udp, };

  class factory
  {
  public:
    virtual ~factory() {};

    virtual int create(Address::Family family, Type type, Protocol protocol,
                       std::unique_ptr<Socket>& Socket) = 0;

  protected:
    factory() {};
  };


  virtual ~Socket() {}

  virtual int bind(const SocketAddress& address) = 0;
  virtual void close() = 0;

  virtual std::size_t recv(char* dst, std::size_t len) = 0;
  virtual int send(const char* src, std::size_t len, const SocketAddress& recipient) = 0;

  virtual int last_error() const = 0;

protected:
  Socket() {}

  virtual int last_error(int error) = 0;

  Address::Family family;
  Type            type;
  Protocol        protocol;

private:
  Socket(const Socket& copy) = delete;
  Socket& operator =(const Socket& copy) = delete;
};

}