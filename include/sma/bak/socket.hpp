#pragma once

#include "inetaddress.hpp"

#include <memory>
#include <cstdlib>
#include <cstdint>



namespace sma
{

class Socket
{
public:
  enum Protocol {
    Udp,
  };

  class factory
  {
  public:
    virtual ~factory(){};

    virtual int create(Protocol protocol, std::unique_ptr<Socket>& Socket) = 0;

  protected:
    factory(){};
  };


  Socket(Socket&& rhs) = default;
  Socket& operator=(Socket&& rhs) = default;
  virtual ~Socket() {}

  virtual int bind(const SocketAddress& address) = 0;
  virtual void close() = 0;

  virtual std::size_t recv(std::uint8_t* dst, std::size_t len) = 0;
  virtual int send(const std::uint8_t* src,
                   std::size_t len,
                   const SocketAddress& recipient) = 0;

  virtual int last_error() const = 0;

protected:
  Socket() {}

  virtual int last_error(int error) = 0;

  Protocol protocol;
};
}
