#pragma once

#include <iostream>
#include <vector>


namespace sma
{

class Address
{
public:
  enum Family { IPv4, };

  virtual void print(std::ostream& os) const;
  friend std::ostream& operator <<(std::ostream& os, const Address& addr);

  const Family                      family;
  const std::vector<unsigned char>  data;

protected:
  Address(const Family& family, const std::vector<unsigned char>& addr);
  Address(const Family& family, std::vector<unsigned char>&& addr);
};

}