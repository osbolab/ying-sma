#pragma once

#include <iosfwd>

namespace sma
{
class ContentStore
{
public:
  virtual ~ContentStore() {}

  StoredContent store(std::istream& is);
};
}
