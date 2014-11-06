#include "inetaddress.hh"

#include <arpa/inet.h>


namespace sma
{

const InetAddress InetAddress::ANY = InetAddress();
const InetAddress InetAddress::LOOPBACK = InetAddress("127.0.0.1");

const std::vector<unsigned char> InetAddress::parse(const std::string& saddr)
{
  const std::size_t len = sizeof(in_addr);
  unsigned char buf[len];
  if (!inet_pton(AF_INET, saddr.c_str(), &buf)) {
    perror("inet_pton");
    return std::vector<unsigned char>();
  }

  return std::vector<unsigned char>(buf, buf + len);
}


void InetAddress::print(std::ostream& os) const
{
  char str[INET_ADDRSTRLEN];
  if (inet_ntop(AF_INET, &data[0], str, INET_ADDRSTRLEN) == NULL) {
    perror("inet_ntop");
  } else {
    os << str;
  }
}
}
