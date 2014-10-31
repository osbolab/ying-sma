#include "nativesocket.hh"

#include "log.hh"

#ifdef WIN32
#include "winsock.hh"
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <cassert>
#include <cstdio>
#include <cerrno>


namespace sma
{

#ifdef WIN32
bool NativeSocket::wsa_is_initialized = false;
#endif


NativeSocket::Factory::Factory()
{
}

int NativeSocket::Factory::create(Address::Family family,
                                  Socket::Type type,
                                  Socket::Protocol protocol,
                                  std::unique_ptr<Socket>& sock_out)
{
  switch (family) {
    case Address::IPv4: {
      auto sock = std::unique_ptr<NativeSocket>(new NativeSocket());
      int error = sock->create(family, type, protocol);
      if (!error) {
        sock_out = std::move(sock);
      }
      return error;
    }

    default:
      return NativeSocket::global_last_error(EAFNOSUPPORT);
  }
}

NativeSocket::NativeSocket()
  : sock(INVALID_SOCKET)
{
  LOG_D("[NativeSocket::()]");
  is_blocking(true);
}

int NativeSocket::create(Address::Family family, Type type, Protocol protocol)
{
  LOG_D("[NativeSocket::create]");

#ifdef WIN32
  if (!wsa_is_initialized) {
    WORD version_wanted = MAKEWORD(2, 2);
    WSADATA wsa_data;
    if (WSAStartup(version_wanted, &wsa_data) != NO_ERROR)
      return -1;
    wsa_is_initialized = true;
    LOG_D("[::WSAStartup] " << wsa_is_initialized);
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

  if (sock != INVALID_SOCKET)
    close();

  sock = socket(family_i, type_i, protocol_i);
  if (sock == INVALID_SOCKET)
    return last_error();

#ifdef WIN32
  BOOL so_reuse = TRUE;
  if (setsockopt(
          sock, SOL_SOCKET, SO_REUSEADDR, (char*) &so_reuse, sizeof(BOOL)) ==
      SOCKET_ERROR) {
    return -1;
  }
#else
  bool so_reuse = true;

#endif

  this->family = family;
  this->type = type;
  this->protocol = protocol;

  return 0;
}

NativeSocket::~NativeSocket()
{
  close();
}

int NativeSocket::bind(const SocketAddress& address)
{
  LOG_D("[NativeSocket::bind] " << address);

  if (address.addr.family != family) {
    return last_error(EAFNOSUPPORT);
  }
  switch (family) {
    case Address::IPv4: {
      const sockaddr sa(address.to_sockaddr());
      if (::bind(sock, &sa, sizeof(sockaddr)) != NO_ERROR)
        return -1;
      return 0;
      break;
    }

    default:
      return last_error(EAFNOSUPPORT);
  }
}

void NativeSocket::close()
{
  LOG_D("[NativeSocket::close]");

  if (sock != INVALID_SOCKET) {
#ifdef WIN32
    int result = closesocket(sock);
#else
    int result = ::close(sock);
#endif
    if (result != NO_ERROR)
      print_last_error();
    sock = INVALID_SOCKET;
  }

#ifdef WIN32
  if (wsa_is_initialized) {
    LOG_D("[::WSACleanup]");
    if (WSACleanup() != NO_ERROR)
      print_last_error();
  }
#endif
}

std::size_t NativeSocket::recv(char* dst, std::size_t len)
{
  LOG_D("[NativeSocket::recv]");

  return ::recv(sock, dst, len, 0);
}

int NativeSocket::send(const char* src,
                       std::size_t len,
                       const SocketAddress& recipient)
{
  LOG_D("[NativeSocket::send]");

  sockaddr sa = recipient.to_sockaddr();
  return ::sendto(sock, src, len, 0, &sa, sizeof(sockaddr));
}

int NativeSocket::is_blocking(bool blocking)
{
#if WIN32
  u_long mode = blocking ? 0 : 1;
  if (ioctlsocket(sock, FIONBIO, &mode) != NO_ERROR)
    return -1;
#else
  int opts;
  if ((opts = fcntl(sock, F_GETFL)) < 0) {
    return -1;
  }
  opts = blocking ? (opts & ~O_NONBLOCK) : (opts | O_NONBLOCK);
  if (fcntl(sock, F_SETFL, opts) < 0) {
    return -1;
  }
#endif
  LOG_D("[NativeSocket::is_blocking] " << (blocking ? "yes" : "no"));

  return 0;
}

bool NativeSocket::is_blocking() const
{
  return blocking;
}

int NativeSocket::last_error() const
{
  return global_last_error();
}

int NativeSocket::global_last_error(int error)
{
#ifdef WIN32
  WSASetLastError(error);
#else
  errno = error;
#endif
  return error;
}

int NativeSocket::global_last_error()
{
#ifdef WIN32
  int error = WSAGetLastError();
#else
  int error = errno;
#endif
#ifdef _DEBUG
  print_last_error();
#endif
  return -1;
}

int NativeSocket::last_error(int error)
{
  return global_last_error(error);
}

void NativeSocket::print_last_error()
{
  int error = NO_ERROR;
  if ((error = global_last_error()) != NO_ERROR) {
#ifdef WIN32
    std::cerr << WSAGetLastErrorMessage("Socket error", error);
#else
    std::perror("Socket error");
#endif
  }
}
}
