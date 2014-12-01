#pragma once

#include <vector>

namespace sma
{
struct ContentType;
struct ContentInfo;

class Interests
{
public:
  virtual ~Interests() {}

  virtual void insert_new(std::vector<ContentType> types) = 0;

  virtual bool interested_in(ContentInfo const& info) const = 0;
  virtual bool know_remote(ContentType const& type) const = 0;
};
}
