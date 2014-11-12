#include <sma/linux/bsd_socket.hpp>
#include <sma/core/abstract_socket.hpp>
#include <sma/core/bytes.hpp>
#include <sma/log.hpp>

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


int bsd_socket::Factory::create(abstract_socket::protocol protocol,
                                  std::unique_ptr<abstract_socket>& sock_out)
{
  LOG(DEBUG) << "protocol: " << protocol;
  auto sock = std::unique_ptr<bsd_socket>(new bsd_socket());
  int error = sock->create(protocol);
  if (!error)
    sock_out = std::move(sock);
  return error;
}

int bsd_socket::create(protocol protocol)
{
  LOG(DEBUG);

  int protocol_i, type_i;
  switch (protocol) {
    case Udp:
      protocol_i = IPPROTO_UDP;
      type_i = SOCK_DGRAM;
      break;
    default:
      return last_error(EPROTONOSUPPORT);
  }

  if (sock != INVALID_SOCKET)
    close();

  sock = socket(AF_INET, type_i, protocol_i);
  if (sock == INVALID_SOCKET)
    return last_error();

  int so_reuse = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &so_reuse, sizeof so_reuse)
      == -1)
    return last_error();

  this->protocol = protocol;

  return 0;
}

int bsd_socket::bind(const socket_addr& address)
{
  LOG(DEBUG) << address;
  auto sa = static_cast<sockaddr>(address);
  if (::bind(sock, &sa, sizeof sa) != NO_ERROR)
    return -1;
  return 0;
}

void bsd_socket::close()
{
  LOG(DEBUG);

  if (sock != INVALID_SOCKET) {
    int result = ::close(sock);
    if (result != NO_ERROR)
      log_last_error();
    sock = INVALID_SOCKET;
  }
}

int bsd_socket::send(const std::uint8_t* src,
                       std::size_t len,
                       const socket_addr& recipient)
{
  LOG(DEBUG);

  auto sa = static_cast<sockaddr>(recipient);
  return ::sendto(sock, char_cp(src), len, 0, &sa, sizeof sa);
}

int bsd_socket::is_blocking(bool blocking)
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

void bsd_socket::log_last_error()
{
  int error = NO_ERROR;
  if ((error = global_last_error()) != NO_ERROR)
    LOG(ERROR) << "Socket error: " << std::strerror(error);
}
}
