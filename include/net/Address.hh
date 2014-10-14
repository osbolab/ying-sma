#ifndef ADDRESS_H_
#define ADDRESS_H_

#include <iostream>
#include <cstdint>
#include <vector>


namespace sma
{

enum AddressType
{
  ADDR_IPV4
};

class Address
{
public:
  virtual void print(std::ostream& os) const = 0;
  friend std::ostream& operator << (std::ostream& os, const Address& addr);

protected:
  Address(AddressType type, std::size_t len, const std::vector<std::uint8_t> addr);

  const std::vector<std::uint8_t> addr;
  const AddressType               type;
  const std::size_t               len;
};

}

#endif