#include <cassert>
#include <cstdio>
#include <cerrno>

#include "net/BsdSocket.hh"


namespace sma
{

#ifdef WIN32
bool BsdSocket::wsaInitialized = false;
#endif

BsdSocket::BsdSocket() : sock(INVALID_SOCKET) {}

int BsdSocket::create(Address::Family family , Type type, Protocol protocol)
{
#ifdef WIN32
  if (!wsaInitialized) {
    WORD versionWanted = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (!WSAStartup(versionWanted, &wsaData)) {
      return getLastError();
    }
    wsaInitialized = true;
  }
#endif

  int iFamily, iType, iProtocol;
  switch (family) {
  case Address::IPv4: iFamily = AF_INET; break;
  default:            return setLastError(EAFNOSUPPORT);
  }
  switch (type) {
  case Datagram:      iType = SOCK_DGRAM; break;
  default:            return setLastError(EPROTOTYPE);
  }
  switch (protocol) {
  case Udp:           iProtocol = IPPROTO_UDP; break;
  default:            return setLastError(EPROTONOSUPPORT);
  }

  if (sock != INVALID_SOCKET) {
    close();
  }

  sock = socket(iFamily, iType, iProtocol);
  if (sock == INVALID_SOCKET) {
    return getLastError();
  }

  return 0;
}

BsdSocket::~BsdSocket()
{
  close();
}

int BsdSocket::bind(const SocketAddress& address)
{
  if (address.addr.family != family) {
    return setLastError(EAFNOSUPPORT);
  }
  switch (family) {
  case Address::IPv4:
    if (::bind(sock, (sockaddr*) &address, sizeof(sockaddr)) != 0) {
      return getLastError();
    }
    return 0;
    break;

  default:
    return setLastError(EAFNOSUPPORT);
  }
}

void BsdSocket::close()
{
  if (sock != INVALID_SOCKET) {
#ifdef WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
    sock = INVALID_SOCKET;
  }
}

std::unique_ptr<Packet> BsdSocket::recv()
{
  return nullptr;
}

int BsdSocket::send(std::unique_ptr<const Packet> packet, const SocketAddress& recipient)
{
  return -1;
}

int BsdSocket::getLastError() const
{
#ifndef NDEBUG
  std::perror("Socket error");
#endif
#ifdef WIN32
  return WSAGetLastError();
#else
  return errno;
#endif
}

int BsdSocket::setLastError(int error)
{
#ifdef WIN32
  WSASetLastError(error);
#else
  errno = error;
#endif
  return error;
}

}