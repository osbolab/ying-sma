#include <cassert> 
#include <utility>

#include "net/Ipv4Address.hh"


namespace sma
{

Ipv4Address::Ipv4Address(const std::string& saddr)
  : Address(ADDR_IPV4,
  ADDR_LENGTH,
  parse(saddr, ADDR_LENGTH)
  )
{
}


const std::vector<std::uint8_t> Ipv4Address::parse(const std::string& saddr, size_t len)
{
  std::vector<std::uint8_t> addr;

  size_t saddrLen = saddr.length();
  unsigned int value = 0;
  for (char c : saddr) {
    if (c == '.') {
      addr.push_back(value);
      value = 0;
    } else {
      c -= '0';
      assert(0 <= c && c < 10 && "Invalid character in IPv4 address");
      value *= 10;
      value += c;
    }
  }
  addr.push_back(value);

  assert(addr.size() == len && "Improperly formatted IPv4 address");

  return std::move(addr);
}


void Ipv4Address::print(std::ostream& os) const
{
  os << (int)addr[0];
  for (std::size_t i = 1; i < len; ++i) {
    os << '.' << (int)addr[i];
  }
}

}