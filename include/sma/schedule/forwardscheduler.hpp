#pragma once

#include <sma/ccn/interest.hpp>
#include <sma/ccn/contentmetadata.hpp>
#include <sma/ccn/ccnfwd.hpp>

namespace sma
{

class ForwardScheduler
{
public:
  virtual void on_interest <std::vector<Interest>> = 0;
  virtual void on_metadata <std::vector<ContentMetadata>> = 0;
  virtual void on_blockrequest <std::vector<BlockRequest>> = 0;
  virtual void on_blockresponse <std::vector<BlockResponse>> = 0;

  virtual ~ForwardScheduler() {}
};
}
