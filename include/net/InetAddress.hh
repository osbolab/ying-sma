#pragma once

#include "Address.hh"

namespace sma
{

class InetAddress final : public Address
{
public:
  static const InetAddress ANY;

  InetAddress(const std::string& addr);

  void print(std::ostream& os) const override;

private:
  static const std::vector<unsigned char> parse(const std::string& saddr);

  InetAddress();
};

}