#pragma once

#include "Socket.hh"

#ifdef WIN32
  #include "Winsock.hh"
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  namespace sma
  {
    typedef int SOCKET;
  }
#endif

#ifndef INVALID_SOCKET
  #define INVALID_SOCKET 0 
#endif


namespace sma
{

class BsdSocket : public Socket
{
  friend class BsdSocketFactory;

public:
  ~BsdSocket();

  int bind(const SocketAddress& address) override;
  void close() override;

  std::unique_ptr<Packet> recv() override;
  int send(std::unique_ptr<const Packet> packet, const SocketAddress& recipient) override;

  int getLastError() const override;

private:
  BsdSocket();

  int create(Address::Family family, Type type, Protocol protocol);

  int setLastError(int error) override;

#ifdef WIN32
  static bool wsaInitialized;
#endif

  SOCKET sock;
};

}