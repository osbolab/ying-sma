#pragma once

#include <sma/messagewriter.hpp>
#include <sma/buffer.hpp>

#include <cstdint>


namespace sma
{
class BinaryMessageWriter final : public MessageWriter
{
public:
  BinaryMessageWriter(void* dst, std::size_t size);
  BinaryMessageWriter(std::size_t size);

  std::size_t write(Message const& msg) override;

private:
  Buffer dst;
};
}
