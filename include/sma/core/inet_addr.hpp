#pragma once

#include <ostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>


namespace sma
{

struct inet_addr final {
  friend struct socket_addr;

  static const inet_addr ANY;
  static const inet_addr LOOPBACK;

  inet_addr() {}
  inet_addr(const std::string& saddr)
  {
    if (!inet_pton(AF_INET, saddr.c_str(), &addr)) {
      perror("inet_pton");
    }
  }

  operator in_addr() const { return addr; }

  friend std::ostream& operator<<(std::ostream& os, const inet_addr& addr);

private:
  in_addr addr{INADDR_ANY};
};

struct socket_addr final {
  socket_addr(std::string ipaddress, std::uint16_t port)
    : addr(ipaddress)
    , port(port)
  {
    saddr.sin.sin_family = AF_INET;
    saddr.sin.sin_port = htons(port);
    std::memset(saddr.sin.sin_zero, 0, 8);
    saddr.sin.sin_addr = static_cast<in_addr>(addr);
  }

  operator sockaddr() const { return saddr.s; }
  operator sockaddr_in() const { return saddr.sin; }
  operator in_addr() const { return saddr.sin.sin_addr; }

  friend std::ostream& operator<<(std::ostream& os, const socket_addr& addr);

  inet_addr addr;
  std::uint16_t port;

private:
  union sa {
    sockaddr s;
    sockaddr_in sin;
  };
  sa saddr;
};

inline std::ostream& operator<<(std::ostream& os, const inet_addr& addr)
{
  char str[16];
  if (inet_ntop(AF_INET, &(addr.addr), str, INET_ADDRSTRLEN) == NULL) {
    perror("inet_ntop");
  } else {
    os << str;
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const socket_addr& addr)
{
  os << addr.addr << ":" << addr.port;
  return os;
}
}
