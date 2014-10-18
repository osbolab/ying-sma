#include <cassert>
#include <cstdio>
#include <cerrno>

#include "net/NativeSocket.hh"


namespace sma
{

#ifdef WIN32
bool NativeSocket::wsaInitialized = false;
#endif

NativeSocket::NativeSocket() : sock(INVALID_SOCKET) 
{
  isBlocking(true);
}

int NativeSocket::create(Address::Family family , Type type, Protocol protocol)
{
#ifdef WIN32
  if (!wsaInitialized) {
    WORD versionWanted = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (!WSAStartup(versionWanted, &wsaData)) {
      return lastError();
    }
    wsaInitialized = true;
  }
#endif

  int iFamily, iType, iProtocol;
  switch (family) {
  case Address::IPv4: iFamily = AF_INET; break;
  default:            return lastError(EAFNOSUPPORT);
  }
  switch (type) {
  case Datagram:      iType = SOCK_DGRAM; break;
  default:            return lastError(EPROTOTYPE);
  }
  switch (protocol) {
  case Udp:           iProtocol = IPPROTO_UDP; break;
  default:            return lastError(EPROTONOSUPPORT);
  }

  if (sock != INVALID_SOCKET) {
    close();
  }

  sock = socket(iFamily, iType, iProtocol);
  if (sock == INVALID_SOCKET) {
    return lastError();
  }

  return 0;
}

NativeSocket::~NativeSocket()
{
  close();
}

int NativeSocket::bind(const SocketAddress& address)
{
  if (address.addr.family != family) {
    return lastError(EAFNOSUPPORT);
  }
  switch (family) {
  case Address::IPv4:
    if (::bind(sock, (sockaddr*) &address, sizeof(sockaddr)) != 0) {
      return lastError();
    }
    return 0;
    break;

  default:
    return lastError(EAFNOSUPPORT);
  }
}

void NativeSocket::close()
{
  if (sock != INVALID_SOCKET) {
#ifdef WIN32
    closesocket(sock);
#else
    close(sock);
#endif
    sock = INVALID_SOCKET;
  }

#ifdef WIN32
  if (wsaInitialized) {
    WSACleanup();
  }
#endif
}

std::unique_ptr<Packet> NativeSocket::recv()
{
  return nullptr;
}

int NativeSocket::send(std::unique_ptr<const Packet> packet, const SocketAddress& recipient)
{
  return -1;
}

int NativeSocket::isBlocking(bool blocking)
{
#if WIN32
  u_long mode = blocking ? 0 : 1;
  ioctlsocket(sock, FIONBIO, &mode);
#else
  int opts;
  if ((opts = fcntl(sock, F_GETFL) < 0) {
    return getLastError();
  }
  opts = (opts | O_NONBLOCK);
  if (fcntl(sock, F_SETFL, opts) < 0) {
    return getLastError();
  }
#endif
  return 0;
}

bool NativeSocket::isBlocking() const
{
  return blocking;
}

int NativeSocket::lastError() const
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

int NativeSocket::lastError(int error)
{
#ifdef WIN32
  WSASetLastError(error);
#else
  errno = error;
#endif
  return error;
}

}