#pragma once

#include <sma/ccn/blockref.hpp>
#include <sma/ccn/blockdata.hpp>
#include <sma/ccn/contentmetadata.hpp>
#include <sma/util/hash.hpp>

#include <iosfwd>
#include <memory>
#include <utility>
#include <cstdlib>
#include <unordered_map>


namespace sma
{
class ContentCache
{
public:
  using block_map = std::unordered_map<BlockIndex, BlockData>;

  ~ContentCache();

  BlockData* find(BlockRef ref);

  block_map* find(Hash hash);
  block_map& find_or_allocate(Hash hash);

  std::pair<Hash, std::size_t> load(std::istream& in, std::size_t block_size);

  bool validate_data(ContentMetadata const& metadata) const;

  std::vector<BlockIndex> missing_blocks(ContentMetadata const& metadata) const;

private:
  std::unordered_map<Hash, block_map> content;
};
}
