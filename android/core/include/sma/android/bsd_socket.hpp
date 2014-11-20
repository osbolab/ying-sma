#pragma once

#include <sma/socket.hpp>
#include <sys/socket.h>

#include <type_traits>
#include <cerrno>


namespace sma
{

class bsd_socket final : public socket
{
public:
  bsd_socket(protocol proto);
  ~bsd_socket();

  void bind(const socket_addr& address) override;
  void close() override;

  std::size_t recv(std::uint8_t* dst, std::size_t len) override;
  int send(const std::uint8_t* src,
           std::size_t len,
           const socket_addr& recipient) override;

  inline socket_type native_socket() const noexcept;

  void blocking(bool block);
  inline bool blocking() const noexcept;

private:
  int sock;
  bool is_blocking{false};
};
}
