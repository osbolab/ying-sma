#pragma once

#include <cstdint>


namespace sma
{
struct Message;

class BinaryMessageReader final
{
public:
  static Message read(std::uint8_t const* src, std::size_t size);

private:
  BinaryMessageReader();
  BinaryMessageReader(BinaryMessageReader&&);
  void operator=(BinaryMessageReader&&);
};
}
