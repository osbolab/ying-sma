#pragma once

#include "socket.hh"

#ifdef WIN32
#include "winsock.hh"
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

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

  std::size_t recv(char* dst, std::size_t len) override;
  int send(const char* src,
           std::size_t len,
           const SocketAddress& recipient) override;

  int is_blocking(bool blocking);
  bool is_blocking() const;

  int last_error() const override;

private:
  NativeSocket();

  int create(Address::Family family, Type type, Protocol protocol);

  int last_error(int error) override;
  static void print_last_error();
  static int global_last_error(int error);
  static int global_last_error();


#ifdef WIN32
  static bool wsa_is_initialized;
#endif

  SOCKET sock;
  bool blocking;
};
}
