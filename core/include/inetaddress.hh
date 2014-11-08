#pragma once

#include <ostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#define ARPA_INET_H_


namespace sma
{

struct InetAddress final {
  static const InetAddress ANY;
  static const InetAddress LOOPBACK;

  InetAddress()
  {
  }
  InetAddress(const std::string& saddr)
  {
    if (!inet_pton(AF_INET, saddr.c_str(), &addr)) {
      perror("inet_pton");
    }
  }

  sockaddr saddr(std::uint16_t port)
  {
    union sad {
      sockaddr sa;
      sockaddr_in sin;
    };
    sad sa;

    sa.sin.sin_family = AF_INET;
    sa.sin.sin_port = htons(port);
    std::memset(sa.sin.sin_zero, 0, 8);
    sa.sin.sin_addr = addr;
    return sa.sa;
  }

  friend std::ostream& operator<<(std::ostream& os, const InetAddress& addr);

  in_addr addr{INADDR_ANY};
};

inline std::ostream& operator<<(std::ostream&os, const InetAddress& addr)
{
  char str[16];
  if (inet_ntop(AF_INET, &(addr.addr), str, INET_ADDRSTRLEN) == NULL) {
    perror("inet_ntop");
  } else {
    os << str;
  }
  return os;
}

}
