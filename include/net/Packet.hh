#pragma once

#include <cstdlib>
#include <cstdint>
#include <vector>

#include "Address.hh"

namespace sma
{

class Packet
{
public:
  Packet(std::size_t size);

private:
  const Address&            sender;
  const Address&            recipient;
  std::vector<std::uint8_t> data;
};

}