#pragma once

#include <vector>

namespace sma
{
struct ContentType;

class Interests
{
public:
  virtual void insert_new(std::vector<ContentType> types) = 0;
};
}
