#pragma once

#include <sma/ccn/blockdata.hpp>
#include <sma/util/hash.hpp>

#include <memory>
#include <iosfwd>
#include <unordered_map>


namespace sma
{
class ContentCache
{
public:
  using size_type = BlockData::size_type;
  using block_index = BlockData::index_type;
  using block_ptr = std::unique_ptr<BlockData>;
  using block_map = std::unordered_map<block_index, block_ptr>;

  virtual ~ContentCache() {}

  virtual block_map* find(Hash hash) = 0;
  // virtual StoredContent* create(Hash hash) = 0;

  virtual Hash load(std::istream& in, size_type block_size) = 0;
};
}
