#pragma once

#include <sma/util/hash.hpp>

#include <iosfwd>
#include <utility>


namespace sma
{
class StoredContent;

class ContentStore
{
public:
  virtual ~ContentStore() {}

  virtual StoredContent* find(Hash hash) = 0;

  virtual StoredContent* store_from(std::istream& in, std::uint32_t block_size)
      = 0;
};
}
