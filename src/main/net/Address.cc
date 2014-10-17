#include <iomanip>

#include "net/Address.hh"


namespace sma
{

Address::Address(const Family& family, const std::vector<unsigned char>& addr)
  : family(family), addr(addr) {}

Address::Address(const Family& family, std::vector<unsigned char>&& addr)
  : family(family), addr(std::move(addr)) {}

void Address::print(std::ostream& os) const
{
  os << "0x" << std::hex << std::setfill('0');
  for (auto c : addr) {
    os << std::setw(2) << c;
  }
  os << std::dec;
}

std::ostream& operator << (std::ostream& os, const Address& addr)
{
  addr.print(os);
  return os;
}

}
