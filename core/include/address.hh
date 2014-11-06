#pragma once

#include <iostream>
#include <vector>
#include <cstdint>


namespace sma
{

class Address
{
public:
  enum Family {
    IPv4,
  };

  virtual void print(std::ostream& os) const;
  friend inline std::ostream& operator<<(std::ostream& os, const Address& addr)
  {
    addr.print(os);
    return os;
  }


  bool operator==(const Address& rhs) const
  {
    return family == rhs.family && data == rhs.data;
  }
  bool operator!=(const Address& rhs) const { return !(*this == rhs); }

  const Family family;
  std::vector<unsigned char> data;

protected:
  Address(Family family, std::uint32_t addrl);
  Address(Family family, const std::vector<unsigned char>& addr)
    : family(family)
    , data(addr)
  {
  }

  Address(Family family, std::vector<unsigned char>&& addr)
    : family(family)
    , data(std::move(addr))
  {
  }
};
}
