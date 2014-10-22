#include <iomanip>

#include "net/Address.hh"


namespace sma
{

Address::Address(const Family& family, const std::vector<unsigned char>& addr)
  : family(family), data(addr) {}

Address::Address(const Family& family, std::vector<unsigned char>&& addr)
  : family(family), data(std::move(addr)) {}

void Address::print(std::ostream& os) const
{
  switch (family) {
    case Family::IPv4:
      os << "IPv4(";
      break;
  }
  os << "0x" << std::hex << std::internal << std::setfill('0');
  for (auto c : data) {
    os << std::setw(2) << (int)c;
  }
  os << std::dec;
  os << ")";
}

std::ostream& operator << (std::ostream& os, const Address& addr)
{
  addr.print(os);
  return os;
}

}
