#pragma once

#include <sma/net/abstract_socket.hpp>
#include <sma/bytes.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET 0
#endif

#ifndef NO_ERROR
#define NO_ERROR 0
#endif


namespace sma
{

#ifndef TYPEDEF_SOCKET_
#define TYPEDEF_SOCKET_
typedef int SOCKET;
#endif

class bsd_socket : public abstract_socket
{
public:
  class Factory : public abstract_socket::factory
  {
  public:
    Factory() {}

    int create(abstract_socket::protocol proto,
               std::unique_ptr<abstract_socket>& sock_out) override;
  };

  friend class Factory;

  ~bsd_socket() { close(); }

  int bind(const SocketAddress& address) override;
  void close() override;

  std::size_t recv(std::uint8_t* dst, std::size_t len) override
  {
    return ::recv(sock, char_p(dst), len, 0);
  }

  int send(const std::uint8_t* src,
           std::size_t len,
           const SocketAddress& recipient) override;

  SOCKET native_socket() const { return sock; }

  int blocking(bool block);
  bool blocking() const { return is_blocking; }

  int last_error() const override { return global_last_error(); }

private:
  bsd_socket() { blocking(true); }

  int create(protocol proto);

  int last_error(int error) override { return global_last_error(error); }

  static void log_last_error();
  static int global_last_error(int error) { return (errno = error); }

  static int global_last_error()
  {
    int error = errno;
    log_last_error();
    return error;
  }

  SOCKET sock{INVALID_SOCKET};
  bool is_blocking;
};
}
