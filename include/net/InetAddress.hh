#pragma once

#include "Address.hh"

namespace sma
{

class InetAddress : public Address {
public:
  InetAddress(const std::string& addr);

  void print(std::ostream& os) const override;

private:
  static const std::vector<unsigned char> parse(const std::string& saddr);
};

}