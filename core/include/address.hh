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
  friend std::ostream& operator<<(std::ostream& os, const Address& addr);

  bool operator==(const Address& rhs) const;
  bool operator!=(const Address& rhs) const;

  const Family family;
  std::vector<unsigned char> data;

protected:
  Address(Family family, std::uint32_t addrl);
  Address(Family family, const std::vector<unsigned char>& addr);
  Address(Family family, std::vector<unsigned char>&& addr);
};
}
