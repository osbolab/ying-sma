#include <sma/ccn/blockdata.hpp>

#include <sma/ccn/contentcache.hpp>

#include <cstring>
#include <cassert>
#include <utility>


namespace sma
{
bool BlockData::exists() const { return cache != nullptr; }


BlockData::operator bool() const { return exists(); }


bool BlockData::complete() const
{
  if (cache != nullptr)
    return cache->slots[idx].size == cache->slots[idx].expected_size;
  else
    return false;
}


std::uint8_t* BlockData::data()
{
  assert(exists());
  return cache->slots[idx].data;
}


std::uint8_t const* BlockData::cdata() const
{
  assert(exists());
  return cache->slots[idx].data;
}


bool BlockData::operator==(BlockData const& rhs) const
{
  return cache == rhs.cache && idx == rhs.idx;
}


bool BlockData::operator!=(BlockData const& rhs) const
{
  return !(*this == rhs);
}


BlockData::BlockData()
  : cache(nullptr)
{
}


BlockData::BlockData(ContentCache* cache, std::size_t idx)
  : cache(*cache)
  , idx(idx)
{
}


std::size_t
BlockData::read(std::uint8_t* dst, std::size_t from, std::size_t size) const
{
  assert(from <= this->size);
  if (from + size > this->size)
    size = this->size - from;

  std::memcpy(dst, data + from, size);
  return size;
}


std::size_t BlockData::read(std::uint8_t* dst, std::size_t size) const
{
  return read(dst, 0, size);
}

#if 0
void BlockData::insert(size_type dst_off,
                       std::uint8_t const* src,
                       size_type size)
{
  assert(dst_off + size <= this->size);
  std::memcpy(data + dst_off, src, size);

  std::vector<size_type> new_gaps;
  auto i_l = dst_off;
  auto i_r = i_l + size - 1;

  for (std::size_t i = 0; i < gaps.size() - 1; i += 2) {
    auto g_l = gaps[i];
    auto g_r = gaps[i + 1];

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

  gaps.swap(new_gaps);
}
#endif
}
