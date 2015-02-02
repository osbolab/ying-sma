#include <sma/ccn/storedcontent.hpp>


namespace sma
{
StoredContent::StoredContent(std::uint32_t size, std::uint32_t block_size)
  : size_(size)
  , block_size_(block_size)
{
}

bool StoredContent::complete() const
{
  for (std::size_t i = 0; i < blocks_count(); ++i) {
    auto block = this->cblock(i);
    if (not block or not block->complete())
      return false;
  }
  return true;
}

std::vector<std::uint32_t> StoredContent::missing_blocks() const
{
  std::vector<std::uint32_t> blocks;
  for (std::uint32_t i = 0; i < blocks_count(); ++i) {
    auto block = this->cblock(i);
    if (not block or not block->complete())
      blocks.push_back(i);
  }
  return blocks;
}
}
