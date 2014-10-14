#include "net/Address.hh"

namespace sma
{

Address::Address(AddressType type, std::size_t len, const std::vector<std::uint8_t> addr)
  : type(type), len(len), addr(addr)
{
}

std::ostream& operator << (std::ostream& os, const Address& addr)
{
  addr.print(os);
  return os;
}

}
