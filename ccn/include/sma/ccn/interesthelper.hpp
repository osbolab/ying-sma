#pragma once

#include <sma/ccn/contenttype.hpp>

#include <sma/io/log>

#include <map>

namespace sma
{
struct Interest;
struct InterestMessage;

class InterestHelper
{
public:
  using hop_count = std::uint32_t;
  using interest_table = std::map<ContentType, Interest>;
  using iterator = interest_table::iterator;

  InterestHelper(Logger log);
  void receive(InterestMessage const& msg);

private:
  Logger log;
};
}
