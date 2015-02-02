#pragma once

#include <cstdint>

namespace sma
{
class ContentDataBlock
{
public:
  virtual ~ContentDataBlock() {}

  std::size_t index() const { return index_; }

  virtual bool complete() const = 0;
  virtual std::size_t size() const = 0;
  virtual std::size_t read(std::uint8_t* dst, std::size_t size) = 0;

private:
  std::size_t index_;
};
}
