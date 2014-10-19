#include "net/NativeSocket.hh"

#include <cassert>
#include <cstdio>
#include <cerrno>


namespace sma
{

#ifdef WIN32
bool NativeSocket::wsa_is_initialized = false;
#endif

NativeSocket::NativeSocket() : sock(INVALID_SOCKET)
{
  is_blocking(true);
}

int NativeSocket::create(Address::Family family , Type type, Protocol protocol)
{
  #ifdef WIN32
  if (!wsa_is_initialized) {
    WORD version_wanted = MAKEWORD(2, 2);
    WSADATA wsa_data;
    if (!WSAStartup(version_wanted, &wsa_data)) {
      return last_error();
    }
    wsa_is_initialized = true;
  }
  #endif

  int family_i, type_i, protocol_i;
  switch (family) {
    case Address::IPv4:
      family_i = AF_INET;
      break;
    default:
      return last_error(EAFNOSUPPORT);
  }
  switch (type) {
    case Datagram:
      type_i = SOCK_DGRAM;
      break;
    default:
      return last_error(EPROTOTYPE);
  }
  switch (protocol) {
    case Udp:
      protocol_i = IPPROTO_UDP;
      break;
    default:
      return last_error(EPROTONOSUPPORT);
  }

  if (sock != INVALID_SOCKET) {
    close();
  }

  sock = socket(family_i, type_i, protocol_i);
  if (sock == INVALID_SOCKET) {
    return last_error();
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
    return last_error(EAFNOSUPPORT);
  }
  switch (family) {
    case Address::IPv4:
      if (::bind(sock, (sockaddr*) &address, sizeof(sockaddr)) != 0) {
        return last_error();
      }
      return 0;
      break;

    default:
      return last_error(EAFNOSUPPORT);
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
  if (wsa_is_initialized) {
    WSACleanup();
  }
  #endif
}

std::size_t NativeSocket::recv(std::uint8_t& dst, std::size_t len)
{
  return 0;
}

int NativeSocket::send(const std::uint8_t& src, std::size_t len, const SocketAddress& recipient)
{
  return -1;
}

int NativeSocket::is_blocking(bool blocking)
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

bool NativeSocket::is_blocking() const
{
  return blocking;
}

int NativeSocket::last_error() const
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

int NativeSocket::last_error(int error)
{
  #ifdef WIN32
  WSASetLastError(error);
  #else
  errno = error;
  #endif
  return error;
}


int NativeSocket::Factory::create(Address::Family family, Socket::Type type,
                                  Socket::Protocol protocol,
                                  std::unique_ptr<Socket>& Socket)
{
  switch (family) {
    case Address::IPv4: {
      auto ptr = std::unique_ptr<NativeSocket>(new NativeSocket());
      int error = ptr->create(family, type, protocol);
      if (!error) {
        Socket = std::move(ptr);
      }
      return error;
    }

    default:
      return EAFNOSUPPORT;
  }
}

}