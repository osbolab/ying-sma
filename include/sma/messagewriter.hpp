#pragma once

#include <cstdint>

namespace sma
{
class MessageWriter
{
public:
  virtual ~MessageWriter() {}

  virtual std::size_t write(Message const& msg) = 0;
};
}
