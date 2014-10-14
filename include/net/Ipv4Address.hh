#ifndef IPV4_ADDRESS_H_
#define IPV4_ADDRESS_H_

#include "Address.hh"

namespace sma
{

class Ipv4Address : public Address {
public:
  Ipv4Address(const std::string& addr);

  void print(std::ostream& os) const;

private:
  static const std::vector<std::uint8_t> parse(const std::string& saddr, std::size_t len);

  static const size_t ADDR_LENGTH = 4;
};

}

#endif