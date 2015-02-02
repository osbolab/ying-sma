#pragma once

#include <sma/util/hash.hpp>
#include <sma/ccn/storedblock.hpp>

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <utility>


namespace sma
{
class ContentStore;


class StoredContent
{
public:
  StoredContent(std::uint32_t size, std::uint32_t block_size);
  virtual ~StoredContent() {}

  virtual std::size_t read(std::uint8_t* dst, std::size_t size) const = 0;

  virtual StoredBlock* block(std::uint32_t index) = 0;
  virtual StoredBlock const* cblock(std::uint32_t index) const = 0;

  virtual StoredBlock& create_block(std::uint32_t index, std::uint32_t size)
      = 0;

  bool complete() const;
  std::vector<std::uint32_t> missing_blocks() const;

  std::uint32_t size() const { return size_; }
  std::uint32_t block_size() const { return block_size_; }
  std::size_t blocks_count() const { return 1 + ((size_ - 1) / block_size_); }

private:
  std::uint32_t size_;
  std::uint32_t block_size_;
};
}
