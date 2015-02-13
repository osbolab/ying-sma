#include <sma/linux/bsd_socket.hpp>
#include <sma/net/socket.hpp>
#include <sma/bytes.hpp>
#include <sma/log>

// BSD sockets stuff
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <cassert>
#include <cstdio>
#include <cerrno>


namespace sma
{

const socket_type bsd_socket::EMPTY = 0;
const socket_error_type bsd_socket::NO_ERROR = 0;


bsd_socket::bsd_socket(protocol protocol)
{
  int protocol_i, type_i;
  switch (protocol) {
    case Udp:
      protocol_i = IPPROTO_UDP;
      type_i = SOCK_DGRAM;
      break;
    default:
      errno = EPROTONOSUPPORT;
      throw_last_error();
  }

  if (sock != EMPTY)
    close();

  sock = socket(AF_INET, type_i, protocol_i);
  if (sock == EMPTY)
    throw_last_error();

  int so_reuse = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &so_reuse, sizeof so_reuse)
      == -1)
    throw_last_error();

  blocking(true);
  this->protocol = protocol;
}
bsd_socket::~bsd_socket() { close(); }


int bsd_socket::bind(const socket_addr& address)
{
  LOG(DEBUG) << address;
  auto sa = static_cast<sockaddr>(address);
  if (::bind(sock, &sa, sizeof sa) != NO_ERROR)
    throw_last_error();
  return 0;
}

std::size_t bsd_socket::recv(std::uint8_t* dst, std::size_t len)
{
  return ::recv(sock, char_p(dst), len, 0);
}

int bsd_socket::send(const std::uint8_t* src,
                     std::size_t len,
                     const socket_addr& recipient)
{
  auto sa = static_cast<sockaddr>(recipient);
  return ::sendto(sock, char_cp(src), len, 0, &sa, sizeof sa);
}

void bsd_socket::close()
{
  if (sock != EMPTY) {
    if (::close(sock) != NO_ERROR)
      throw_last_error();
    sock = EMPTY;
  }
}

void bsd_socket::blocking(bool block)
{
  int opts;
  if ((opts = fcntl(sock, F_GETFL)) < 0)
    throw_last_error();
  opts = block ? (opts & ~O_NONBLOCK) : (opts | O_NONBLOCK);
  if (fcntl(sock, F_SETFL, opts) < 0)
    throw_last_error();

  is_blocking = block;
}

void bsd_socket::throw_last_error()
{
  int error = errno;
  if (error != NO_ERROR) {
    LOG(ERROR) << "Socket error: " << std::strerror(error);
    throw socket_exception(error);
  }
}

SOCKET native_socket() const { return sock; }
bool bsd_socket::blocking() const { return is_blocking; }
}
