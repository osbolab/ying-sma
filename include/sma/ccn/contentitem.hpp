#pragma once

#include <sma/ccn/contentdatablock.hpp>

#include <cstdint>
#include <utility>


namespace sma
{
struct ContentItem
{
  std::size_t blocks_count() const;
  std::pair<bool, ContentDataBlock> block(std::size_t index);

  ContentMetadata metadata;
};
}
