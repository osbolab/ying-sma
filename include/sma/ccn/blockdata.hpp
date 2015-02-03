#pragma once

#include <cstdlib>
#include <cstdint>
#include <vector>


namespace sma
{
struct BlockData {
  using index_type = std::size_t;
  using size_type = std::size_t;

  BlockData(index_type index, size_type size);
  BlockData(index_type index, size_type size, std::uint8_t const* src);
  ~BlockData();

  BlockData(BlockData&& rhs);
  BlockData& operator=(BlockData&& rhs);

  BlockData(BlockData const&) = delete;
  BlockData& operator=(BlockData const&) = delete;

  bool complete() const { return gaps.empty(); }

  size_type read(std::uint8_t* dst, size_type from, size_type size) const;
  size_type read(std::uint8_t* dst, size_type size) const;

  void insert(size_type dst_off, std::uint8_t const* src, size_type size);


  index_type index;
  size_type size;
  std::uint8_t* data;
  bool notified{false};

  // Index of start of gap (inclusive) and end of gap (exclusive)
  std::vector<size_type> gaps;
};
}
