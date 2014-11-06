#include "nativesocket.hh"
#include "socket.hh"

#include "log.hh"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <cassert>
#include <cstdio>
#include <cerrno>


namespace sma
{


int NativeSocket::Factory::create(Address::Family family,
                                  Socket::Type type,
                                  Socket::Protocol protocol,
                                  std::unique_ptr<Socket>& sock_out)
{
  LOG(DEBUG) << "Family: " << family << " Type: " << type
             << " Protocol: " << protocol;
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

int NativeSocket::create(Address::Family family, Type type, Protocol protocol)
{
  LOG(DEBUG);

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

  int so_reuse = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &so_reuse, sizeof so_reuse)
      == -1)
    return last_error();

  this->family = family;
  this->type = type;
  this->protocol = protocol;

  return 0;
}

int NativeSocket::bind(const SocketAddress& address)
{
  LOG(DEBUG) << address;

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
  LOG(DEBUG);

  if (sock != INVALID_SOCKET) {
    int result = ::close(sock);
    if (result != NO_ERROR)
      log_last_error();
    sock = INVALID_SOCKET;
  }
}

int NativeSocket::send(const std::uint8_t* src,
                       std::size_t len,
                       const SocketAddress& recipient)
{
  LOG(DEBUG);

  sockaddr sa = recipient.to_sockaddr();
  const char* csrc = reinterpret_cast<const char*>(src);
  return ::sendto(sock, csrc, len, 0, &sa, sizeof(sockaddr));
}

int NativeSocket::is_blocking(bool blocking)
{
  int opts;
  if ((opts = fcntl(sock, F_GETFL)) < 0) {
    return -1;
  }
  opts = blocking ? (opts & ~O_NONBLOCK) : (opts | O_NONBLOCK);
  if (fcntl(sock, F_SETFL, opts) < 0) {
    return -1;
  }
  LOG(DEBUG) << (blocking ? "yes" : "no");

  return 0;
}

void NativeSocket::log_last_error()
{
  int error = NO_ERROR;
  if ((error = global_last_error()) != NO_ERROR)
    LOG(ERROR) << "Socket error: " << std::strerror(error);
}
}
