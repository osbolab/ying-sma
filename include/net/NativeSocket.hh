#pragma once

#include "Socket.hh"

#ifdef WIN32
#include "Winsock.hh"
#else
#include <sys/types.h>
#include <sys/Socket.h>
#include <netinet/in.h>
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET 0
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

    int create(Address::Family family, Socket::Type type, Socket::Protocol protocol,
               std::unique_ptr<Socket>& Socket) override;
  };

  ~NativeSocket();

  int bind(const SocketAddress& address) override;
  void close() override;

  std::size_t recv(std::uint8_t& dst, std::size_t len) override;
  int send(const std::uint8_t& src, std::size_t len, const SocketAddress& recipient) override;

  int is_blocking(bool blocking);
  bool is_blocking() const;

  int last_error() const override;

private:
  NativeSocket();

  int create(Address::Family family, Type type, Protocol protocol);

  int last_error(int error) override;

  #ifdef WIN32
  static bool wsa_is_initialized;
  #endif

  SOCKET  sock;
  bool    blocking;
};

}