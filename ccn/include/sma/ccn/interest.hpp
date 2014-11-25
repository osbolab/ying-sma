#pragma once

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/interestrank.hpp>

namespace sma
{
struct Interest
{
  ContentType type;
  InterestRank rank;
};
}
