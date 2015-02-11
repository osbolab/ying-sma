#pragma once

#include <sma/ccn/blockref.hpp>
#include <sma/ccn/contentmetadata.hpp>
#include <sma/util/hash.hpp>

#include <iosfwd>
#include <utility>
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <forward_list>


namespace sma
{
class ContentCache;

class BlockData
{
  bool exists() const { return cache != nullptr; }
  operator bool() const { return exists(); }

  bool complete() const
  {
    if (cache != nullptr)
      return cache->slots[idx].size == cache->slots[idx].expected_size;
    else
      return false;
  }

  std::uint8_t* data()
  {
    assert(exists());
    return cache->slots[idx].data;
  }

  std::uint8_t const* cdata() const
  {
    assert(exists());
    return cache->slots[idx].data;
  }

  bool operator==(BlockData const& rhs) const
  {
    return cache == rhs.cache && idx == rhs.idx;
  }
  bool operator!=(BlockData const& rhs) const { return !(*this == rhs); }

private:
  friend class ContentCache;

  BlockData()
    : cache(nullptr)
  {
  }

  BlockData(ContentCache* cache, std::size_t idx)
    : cache(*cache)
    , idx(idx)
  {
  }

  ContentCache* cache;
  std::size_t idx;
};

class ContentCache
{
  // The size of cache slots and the blocks into which content data are
  // divided.
  static std::size_t const block_size = 1024;

public:
  // If capacity is zero this cache is unbounded and dynamic with no initial
  // size; otherwise it is a bounded LRU allocated at construction.
  ContentCache(std::size_t capacity);
  ~ContentCache();

  BlockData end() const { return BlockData(); }

  // Return the cached block data for the given content block
  BlockData find(BlockRef ref);

  // Return the map of block indices to block data for the given content hash
  // or nullptr if none exists.
  block_map* find(Hash hash);
  // Return the map of block indices to block data for the given content hash;
  // if none exists, create one and allocate the
  block_map& find_or_allocate(Hash hash);

  // Insert a new item of content into the cache by dividing into blocks,
  // hashing, and caching its data.
  std::pair<Hash, std::size_t> load(std::istream& in);

  // Return true if all blocks for the given content exist and are complete.
  bool validate_data(ContentMetadata const& metadata) const;

  // Get the indices of all blocks that are either missing or incomplete
  // for the given content.
  std::vector<BlockIndex> missing_blocks(ContentMetadata const& metadata) const;

private:
  friend class BlockData;

  struct Slot {
    std::uint8_t data[block_size];
    // The actual size of stored data.
    std::size_t size;
    // The size of data that should be stored in this slot.
    std::size_t expected_size;
    // True if this slot can't be evicted or recycled.
    bool frozen{false};
  };

  // The actual data, initially allocated when capacity is nonzero.
  std::vector<Slot> slots;
  std::forward_list<std::size_t> occupied_idxs;
  std::vector<std::size_t> free_idxs;
};
}
