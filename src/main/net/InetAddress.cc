#include <cassert> 
#include <utility>

#ifdef WIN32
  #include "net/Winsock.hh"
#else
  #include <arpa/inet.h>
  #define ARPA_INET_H_
#endif

#include "net/InetAddress.hh"


namespace sma
{

InetAddress::InetAddress(const std::string& saddr)
  : Address(IPv4, parse(saddr)) {}

const std::vector<unsigned char> InetAddress::parse(const std::string& saddr)
{
  const std::size_t len = sizeof(in_addr);
  unsigned char buf[len];
  if (!inet_pton(AF_INET, saddr.c_str(), &buf)) {
    perror("inet_pton");
    return std::move(std::vector<unsigned char>());
  }

  return std::move(std::vector<unsigned char>(buf, buf + len));
}


void InetAddress::print(std::ostream& os) const
{
  char str[INET_ADDRSTRLEN];
  if (inet_ntop(AF_INET, &addr[0], str, INET_ADDRSTRLEN) == NULL) {
    perror("inet_ntop");
  } else {
    os << str;
  }
}

}