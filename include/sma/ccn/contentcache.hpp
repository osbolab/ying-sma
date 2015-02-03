#pragma once

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
  using block_index = BlockData::index_type;
  using block_map = std::unordered_map<block_index, BlockData>;

  ~ContentCache();

  block_map* find(Hash const& hash);
  block_map const* find_const(Hash const& hash) const;
  block_map& find_or_allocate(Hash const& hash);

  std::pair<Hash, std::size_t> load(std::istream& in, std::size_t block_size);

  bool validate_data(ContentMetadata const& metadata) const;

  std::vector<std::size_t> missing_blocks(ContentMetadata const& metadata) const;

private:
  std::unordered_map<Hash, block_map> content;
};
}
