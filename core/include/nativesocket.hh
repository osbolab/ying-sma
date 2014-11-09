#pragma once

#include "socket.hh"
#include "bytes.hh"

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

class NativeSocket : public Socket
{
public:
  class Factory : public Socket::factory
  {
  public:
    Factory() {}

    int create(Socket::Protocol protocol,
               std::unique_ptr<Socket>& sock_out) override;
  };

  friend class Factory;

  ~NativeSocket() { close(); }

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

  int is_blocking(bool blocking);
  bool is_blocking() const { return blocking; }

  int last_error() const override { return global_last_error(); }

private:
  NativeSocket() { is_blocking(true); }

  int create(Protocol protocol);

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
  bool blocking;
};
}
