#pragma once

#include <iosfwd>


namespace sma
{
struct ContentType;
struct ContentName;
struct ContentDescriptor;

class ContentStore
{
public:
  virtual ~ContentStore() {}

  virtual ContentDescriptor
  store(ContentType type, ContentName name, std::istream& is) = 0;
};
}
