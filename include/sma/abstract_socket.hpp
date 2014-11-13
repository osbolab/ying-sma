#pragma once

#include <sma/inet_address.hpp>

#include <memory>
#include <cstdlib>
#include <cstdint>


namespace sma
{

class abstract_socket
{
public:
  enum protocol {
    Udp,
  };

  class factory
  {
  public:
    virtual ~factory(){};

    virtual int create(protocol proto,
                       std::unique_ptr<abstract_socket>& abstract_socket) = 0;

  protected:
    factory(){};
  };


  abstract_socket(abstract_socket&& rhs) = default;
  abstract_socket& operator=(abstract_socket&& rhs) = default;
  virtual ~abstract_socket() {}

  virtual int bind(const socket_addr& address) = 0;
  virtual void close() = 0;

  virtual std::size_t recv(std::uint8_t* dst, std::size_t len) = 0;
  virtual int send(const std::uint8_t* src,
                   std::size_t len,
                   const socket_addr& recipient) = 0;

  virtual int last_error() const = 0;

protected:
  abstract_socket() {}

  virtual int last_error(int error) = 0;
};
}
