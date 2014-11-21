#pragma once

#include <cstdint>


namespace sma
{
struct Message;

class BinaryMessageWriter final
{
public:
  static std::size_t
  write(std::uint8_t* dst, std::size_t size, Message const& msg);

private:
  BinaryMessageWriter();
  BinaryMessageWriter(BinaryMessageWriter&&);
  void operator=(BinaryMessageWriter&&);
};
}
