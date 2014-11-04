#pragma once

#include "socket.hh"

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
    Factory();

    int create(Address::Family family,
               Socket::Type type,
               Socket::Protocol protocol,
               std::unique_ptr<Socket>& sock_out) override;
  };

  friend class Factory;

  ~NativeSocket();

  int bind(const SocketAddress& address) override;
  void close() override;

  std::size_t recv(std::uint8_t* dst, std::size_t len) override;
  int send(const std::uint8_t* src,
           std::size_t len,
           const SocketAddress& recipient) override;

  SOCKET native_socket() const { return sock; }

  int is_blocking(bool blocking);
  bool is_blocking() const;

  int last_error() const override;

private:
  NativeSocket();

  int create(Address::Family family, Type type, Protocol protocol);

  int last_error(int error) override;
  static void log_last_error();
  static int global_last_error(int error);
  static int global_last_error();


  SOCKET sock;
  bool blocking;
};
}
