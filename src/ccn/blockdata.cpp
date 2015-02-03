#include <sma/ccn/blockdata.hpp>

#include <cstring>
#include <cassert>


namespace sma
{
BlockData::BlockData(index_type index, size_type size)
  : index(index)
  , size(size)
  , data(new std::uint8_t[size])
{
  gaps.push_back(0);
  gaps.push_back(size - 1);
}

BlockData::BlockData(index_type index, size_type size, std::uint8_t const* src)
  : index(index)
  , size(size)
  , data(new std::uint8_t[size])
{
  std::memcpy(data, src, size);
}

BlockData::~BlockData() { delete[] data; }

BlockData::BlockData(BlockData&& rhs)
  : index(rhs.index)
  , size(rhs.size)
  , data(rhs.data)
{
  rhs.data = nullptr;
}

BlockData& BlockData::operator=(BlockData&& rhs)
{
  index = rhs.index;
  size = rhs.size;
  data = rhs.data;
  rhs.data = nullptr;
  return *this;
}

BlockData::size_type
BlockData::read(std::uint8_t* dst, size_type from, size_type size) const
{
  assert(from <= this->size);
  if (from + size > this->size)
    size = this->size - from;

  std::memcpy(dst, data + from, size);
  return size;
}

BlockData::size_type BlockData::read(std::uint8_t* dst, size_type size) const
{
  return read(dst, 0, size);
}

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
}
