#pragma once

#include "address.hh"

#include <arpa/inet.h>
#define ARPA_INET_H_


namespace sma
{

class InetAddress final : public Address
{
public:
  static const InetAddress ANY;
  static const InetAddress LOOPBACK;

  InetAddress(const std::string& saddr)
    : Address(IPv4, parse(saddr))
  {
  }

  void print(std::ostream& os) const override;

private:
  static const std::vector<unsigned char> parse(const std::string& saddr);

  InetAddress()
    : Address(IPv4, INADDR_ANY)
  {
  }
};
}
