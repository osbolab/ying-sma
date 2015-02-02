#include <sma/ccn/memorycontentstore.hpp>

#include <sma/io/log>

#include <string>
#include <cstring>
#include <istream>
#include <cassert>


namespace sma
{
/******************************************************************************
 * Block
 */
MemoryContentStore::Block::Block(std::uint32_t index, std::uint32_t size)
  : StoredBlock(index, size)
{
  assert(size > 0);
  data_ = new std::uint8_t[size];
  gaps_.push_back(0);
  gaps_.push_back(size - 1);
}

MemoryContentStore::Block::~Block() { delete[] data_; }


std::size_t MemoryContentStore::Block::read(std::uint8_t* dst,
                                            std::size_t src_off,
                                            std::size_t size) const
{
  assert(src_off + size <= this->size());
  std::memcpy(dst, data_, size);
  return size;
}


void MemoryContentStore::Block::insert(std::size_t dst_off,
                                       std::uint8_t const* src,
                                       std::size_t size)
{
  assert(dst_off + size <= this->size());
  std::memcpy(data_ + dst_off, src, size);

  StoredBlock::gaps_vec new_gaps;
  auto i_l = dst_off;
  auto i_r = i_l + size - 1;

  for (std::size_t i = 0; i < gaps_.size() - 1; i += 2) {
    auto g_l = gaps_[i];
    auto g_r = gaps_[i + 1];

    if (i_l >= g_r || i_r <= g_l) {
      // Gap preserved
      new_gaps.push_back(g_l);
      new_gaps.push_back(g_r);
    } else {
      // Gap filled
      if (i_l <= g_l && i_r >= g_r)
        continue;
      // Split to left
      if (i_l > g_l && i_r >= g_r) {
        new_gaps.push_back(g_l);
        new_gaps.push_back(i_l);
      }
      // Split to right
      if (i_l <= g_l && i_r < g_r) {
        new_gaps.push_back(i_r);
        new_gaps.push_back(g_r);
      }
    }
  }

  gaps_.swap(new_gaps);
}
/* Block
 *****************************************************************************/


/******************************************************************************
 * Record
 */
MemoryContentStore::Record::Record(std::uint32_t size,
                                   std::uint32_t block_size,
                                   std::vector<std::unique_ptr<Block>> blocks)
  : StoredContent(size, block_size)
{
  for (std::uint32_t i = 0; i < blocks.size(); ++i)
    this->blocks.emplace(i, std::move(blocks[i]));
}

std::size_t MemoryContentStore::Record::read(std::uint8_t* dst,
                                             std::size_t size) const
{
  std::size_t read = 0;
  std::size_t block_idx = 0;

  while (read < size && block_idx++ < blocks_count()) {
    auto const& block = cblock(block_idx);
    assert(block != nullptr);
    assert(block->complete());

    auto remaining = size - read;
    if (remaining > block->size())
      remaining = block->size();

    block->read(dst + read, 0, remaining);
    read += remaining;
  }

  return read;
}

StoredBlock* MemoryContentStore::Record::block(std::uint32_t index)
{
  auto it = blocks.find(index);
  if (it != blocks.end())
    return static_cast<StoredBlock*>(it->second.get());
  else
    return nullptr;
}

StoredBlock const* MemoryContentStore::Record::cblock(std::uint32_t index) const
{
  auto it = blocks.find(index);
  if (it != blocks.end())
    return static_cast<StoredBlock*>(it->second.get());
  else
    return nullptr;
}
/* Record
 *****************************************************************************/


StoredContent const* MemoryContentStore::find(Hash hash)
{
  auto it = content.find(hash);
  if (it != content.end())
    return static_cast<StoredContent const*>(it->second.get());
  else
    return nullptr;
}

std::pair<Hash, StoredContent const*>
MemoryContentStore::store_from(std::istream& in, std::uint32_t block_size)
{
  auto hasher = Hasher();

  std::vector<std::unique_ptr<Block>> blocks;
  std::uint32_t total_size = 0;

  std::uint32_t index = 0;
  std::uint32_t size;
  char buffer[block_size];

  while (in) {
    in.read(buffer, sizeof buffer);
    auto const read = in.gcount();

    if (read != 0) {
      blocks.push_back(std::make_unique<Block>(index, read));
      ++index;
      auto& block = *(blocks.back().get());
      std::memcpy(block.data_, buffer, read);
      block.gaps_.clear();

      total_size += read;
      hasher(block.data_, read);
    }
  }

  assert(!blocks.empty());

  auto hash = hasher.digest();
  auto existing = content.find(hash);
  if (existing != content.end()) {
    LOG(WARNING) << "Overwriting duplicate content in local cache";
    content.erase(existing);
  }

  LOG(DEBUG) << "Content stored in local cache:";
  LOG(DEBUG) << "| hash: " << std::string(hash);
  LOG(DEBUG) << "| size: " << total_size;
  LOG(DEBUG) << "| blocks: " << blocks.size();

  auto result = content.emplace(
      hash,
      std::make_unique<Record>(total_size, block_size, std::move(blocks)));

  assert(result.second);
  return std::make_pair(hash, result.first->second.get());
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
