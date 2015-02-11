#pragma once

#include <cstdlib>
#include <cstdint>
#include <vector>


namespace sma
{
class ContentCache;

class BlockData
{
  bool exists() const;
  operator bool() const;

  bool complete() const;

  std::uint8_t* data();
  std::uint8_t const* cdata() const;

  bool operator==(BlockData const& rhs) const;
  bool operator!=(BlockData const& rhs) const;

private:
  friend class ContentCache;

  BlockData();
  BlockData(ContentCache* cache, std::size_t idx);

  ContentCache* cache;
  std::size_t idx;
};
}
