#pragma once

#include <sma/util/hash.hpp>
#include <sma/ccn/contentstore.hpp>

#include <sma/ccn/contentdescriptor.hpp>

#include <cstdint>

#include <unordered_map>

namespace sma
{
struct StoredBlock
{
  static constexpr std::uint32_t BLOCK_SIZE = 1024;

  std::uint8_t data[BLOCK_SIZE];
  //! Amount actually in use.
  std::uint32_t size;

  std::vector<std::pair<std::uint32_t, std::uint32_t>> gaps;
};

struct StoredContent
{
  ContentDescriptor descriptor;
  std::vector<StoredBlock> blocks;
};

class MemoryContentStore : public ContentStore
{
public:
  virtual ContentDescriptor
  store(ContentType type, ContentName name, std::istream& is) override;

private:
  std::unordered_map<Hash, StoredContent> content;
};
}
