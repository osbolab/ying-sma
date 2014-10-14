#ifndef PACKET_H_
#define PACKET_H_

#include <cstdlib>
#include <cstdint>
#include <vector>

namespace sma
{

class Packet
{
public:
  Packet(std::size_t size);

private:
  std::vector<std::uint8_t> data;
};

}

#endif