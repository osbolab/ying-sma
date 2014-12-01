#pragma once

#include <sma/ccn/contenthash.hpp>
#include <sma/ccn/blockhash.hpp>
#include <sma/ccn/blockdata.hpp>

#include <unordered_map>

namespace sma
{
class MemoryContentStore : public ContentStore
{
public:
  virtual StoredContent store(std::istream& is) override;

private:
  using block_map = std::unordered_map<BlockHash, BlockData>;
  std::unordered_map<ContentHash, block_map> content;
};
}
