#include <sma/ccn/memorycontentstore.hpp>

#include <sma/io/log>

#include <string>
#include <cstring>
#include <istream>
#include <cassert>


namespace sma
{
block_map* MemoryContentStore::find(Hash hash)
{
  auto it = content.find(hash);
  if (it != content.end())
    return it->second;
  else
    return nullptr;
}

Hash MemoryContentStore::store_from(std::istream& in, std::uint32_t block_size)
{
  auto hasher = Hasher();

  block_map blocks;
  std::uint32_t total_size = 0;

  std::uint32_t index = 0;
  std::uint32_t size;
  char buffer[block_size];

  while (in) {
    in.read(buffer, sizeof buffer);
    auto const read = in.gcount();

    if (read != 0) {
      auto block = new Block(index, read);
      std::memcpy(block.data_, buffer, read);
      block.gaps_.clear();

      total_size += read;
      hasher(block.data_, read);

      blocks.emplace(
          index++,
          std::unique_ptr<StoredBlock>(static_cast<StoredBlock*>(block)));
    }
  }

  assert(!blocks.empty());

  auto hash = hasher.digest();
  if (content.erase(hash) != 0)
    LOG(WARNING) << "Overwriting duplicate content in local cache";

  LOG(DEBUG) << "Content stored in local cache:";
  LOG(DEBUG) << "| hash: " << std::string(hash);
  LOG(DEBUG) << "| size: " << total_size;
  LOG(DEBUG) << "| blocks: " << blocks.size();

  auto result = content.emplace(hash, std::move(blocks));
  assert(result.second);

  return hash;
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

std::unsorted_map<std::size_t, BlockData::gap_vec_type>
ContentData::missing_blocks() const
{
  std::unsorted_map<std::size_t, BlockData::gap_vec_type> missing;

  for (std::size_t i = 0; i < nblocks; ++i) {
    auto it = blocks.find(i);
    if (it != blocks.end())
      missing.emplace(i, it->second.gaps);
    else {
      BlockData::gap_vec gaps;
      gaps.emplace(0, block_size(i));
      missing.emplace(i, gaps);
    }
  }

  return missing;
}
*/
