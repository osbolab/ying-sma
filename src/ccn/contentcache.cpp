#include <sma/ccn/contentcache.hpp>

#include <sma/io/log>

#include <string>
#include <cstring>
#include <istream>
#include <cassert>


namespace sma
{
ContentCache::~ContentCache() {}

ContentCache::block_map* ContentCache::find(Hash const& hash)
{
  auto it = content.find(hash);
  if (it != content.end())
    return &(it->second);
  else
    return nullptr;
}

ContentCache::block_map const* ContentCache::find_const(Hash const& hash) const
{
  auto it = content.find(hash);
  if (it != content.end())
    return &(it->second);
  else
    return nullptr;
}

ContentCache::block_map& ContentCache::find_or_allocate(Hash const& hash)
{
  auto ptr = find(hash);
  if (ptr)
    return *ptr;
  else {
    auto it = content.emplace(hash, block_map()).first;
    return it->second;
  }
}

std::pair<Hash, std::size_t> ContentCache::load(std::istream& in,
                                                std::size_t block_size)
{
  auto hasher = Hasher();

  block_map blocks;
  std::size_t total_size = 0;

  BlockData::index_type index = 0;
  char buffer[block_size];

  while (in) {
    in.read(buffer, sizeof buffer);
    std::size_t const read = in.gcount();

    if (read != 0) {
      auto block = BlockData(
          index, read, reinterpret_cast<std::uint8_t const*>(buffer));
      hasher(block.data, read);
      blocks.emplace(index++, std::move(block));

      total_size += read;
    }
  }

  assert(!blocks.empty());

  auto hash = hasher.digest();
  if (content.erase(hash) != 0)
    LOG(WARNING) << "Overwriting duplicate content in cache";

  LOG(DEBUG) << "Content cached";
  LOG(DEBUG) << "| hash: " << std::string(hash);
  LOG(DEBUG) << "| size: " << total_size;
  LOG(DEBUG) << "| blocks: " << blocks.size();

  content.emplace(hash, std::move(blocks));

  return std::make_pair(hash, total_size);
}

bool ContentCache::validate_data(ContentMetadata const& metadata) const
{
  auto const blocks = find_const(metadata.hash);
  if (blocks == nullptr)
    return false;

  std::size_t expected_blocks = 1 + ((metadata.size - 1) / metadata.block_size);
  std::size_t last_block_size = metadata.size
                                - metadata.block_size * (expected_blocks - 1);

  for (std::size_t i = 0; i < expected_blocks; ++i) {
    auto block_search = blocks->find(i);
    if (block_search == blocks->end())
      return false;
    auto const& block = block_search->second;
    if (i == expected_blocks - 1)
      assert(block.size == last_block_size);
    else
      assert(block.size == metadata.block_size);
    if (not block.complete())
      return false;
  }

  return true;
}

std::vector<std::size_t>
ContentCache::missing_blocks(ContentMetadata const& metadata) const
{
  std::size_t expected_blocks = 1 + ((metadata.size - 1) / metadata.block_size);
  std::vector<std::size_t> missing;

  auto const blocks = find_const(metadata.hash);
  if (blocks == nullptr) {
    for (std::size_t i = 0; i < expected_blocks; ++i)
      missing.push_back(i);
    return missing;
  }

  for (std::size_t i = 0; i < expected_blocks; ++i) {
    auto block_search = blocks->find(i);
    if (block_search == blocks->end())
      missing.push_back(i);
    else {
      auto const& block = block_search->second;
      if (not block.complete())
        missing.push_back(i);
    }
  }

  return missing;
}
}

/*
std::vector<std::size_t> negate1d(std::vector<std::size_t> segments,
                                  std::size_t max)
{
}

std::vector<std::size_t> intersect1d(std::vector<std::size_t> a,
                                     std::vector<std::size_t> b)
{
  assert(a.size() % 2 == 0);
  assert(b.size() % 2 == 0);

  std::vector<std::size_t> intersection;

  for (std::size_t ai = 0; ai < a.size() - 1; ai += 2) {
    al = a[ai];
    ar = a[ai + 1];
    assert(al < ar);

    for (std::size_t bi = 0; bi < b.size() - 1; bi += 2) {
      bl = b[bi];
      br = b[bi + 1];
      assert(bl < br);

      // Not yet reached our segment
      if (br <= al)
        continue;
      // B will only increase farther past our segment
      if (bl >= ar)
        break;

      if (bl <= al && br >= ar) {
        // A is inside B
        intersection.push_back(al);
        intersection.push_back(ar);
      } else if (al <= bl && ar >= br) {
        // B is inside A
        intersection.push_back(bl);
        intersection.push_back(br);
      }
    }
  }
}
*/
