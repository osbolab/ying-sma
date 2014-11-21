#pragma once

#include <sma/inet_addr.hpp>

#include <memory>
#include <cstdlib>
#include <cstdint>
#include <exception>


namespace sma
{
class socket_exception : public std::exception
{
public:
  socket_exception(int err) noexcept : err(err) {}

private:
  int err;
};

class inet_socket
{
public:
  inet_socket() {}
  inet_socket(inet_socket&& rhs) {}
  inet_socket& operator=(inet_socket&& rhs) { return *this; }
  virtual ~inet_socket() {}

  virtual void bind(const socket_addr& address) = 0;
  virtual void close() = 0;

  virtual std::size_t recv(std::uint8_t* dst, std::size_t len) = 0;
  virtual void
  send(const std::uint8_t* src, std::size_t len, const socket_addr& dest) = 0;
  virtual void broadcast(const std::uint8_t* src, std::size_t len) = 0;
};
}
