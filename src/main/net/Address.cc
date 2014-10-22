#include <iomanip>

#include "net/Address.hh"


namespace sma
{

Address::Address(Family family, std::uint32_t addrl)
  : family(family), data(std::vector<unsigned char>(4))
{
  addrl = addrl;
  data[0] = (unsigned char) (addrl >> 24);
  data[1] = (unsigned char) (addrl >> 16);
  data[2] = (unsigned char) (addrl >> 8);
  data[3] = (unsigned char) (addrl & 0xFF);
}

Address::Address(Family family, const std::vector<unsigned char>& addr)
  : family(family), data(addr) {}

Address::Address(Family family, std::vector<unsigned char>&& addr)
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
