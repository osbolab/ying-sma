#pragma once

#include <iosfwd>

namespace sma
{
struct ContentType;
struct ContentName;

class Content
{
public:
  virtual ~Content() {}

  virtual void publish(ContentType type, ContentName name, std::istream& is) = 0;
};
}
