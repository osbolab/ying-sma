#pragma once

#include "Socket.hh"

#ifdef WIN32
  #include "Winsock.hh"
#else
  #include <sys/types.h>
  #include <sys/socket.h>
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
  friend class NativeSocketFactory;

public:
  ~NativeSocket();

  int bind(const SocketAddress& address) override;
  void close() override;
  
  std::unique_ptr<Packet> recv() override;
  int send(std::unique_ptr<const Packet> packet, const SocketAddress& recipient) override;

  int isBlocking(bool blocking);
  bool isBlocking() const;

  int lastError() const override;

private:
  NativeSocket();

  int create(Address::Family family, Type type, Protocol protocol);

  int lastError(int error) override;

#ifdef WIN32
  static bool wsaInitialized;
#endif

  SOCKET  sock;
  bool    blocking;
};

}