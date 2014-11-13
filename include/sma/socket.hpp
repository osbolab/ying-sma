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

class socket
{
public:
  enum protocol {
    UDP,
  };

  socket() {}
  socket(socket&& rhs) noexcept {}
  socket& operator=(socket&& rhs) noexcept {}
  virtual ~socket() noexcept {}

  virtual void bind(const socket_addr& address) = 0;
  virtual void close() = 0;

  virtual std::size_t recv(std::uint8_t* dst, std::size_t len) = 0;
  virtual void
  send(const std::uint8_t* src, std::size_t len, const socket_addr& dest) = 0;
};
}
