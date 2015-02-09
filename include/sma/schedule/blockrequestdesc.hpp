#pragma once

#include <sma/util/vec2d.hpp>
#include <sma/util/hash.hpp>

namespace sma
{
struct BlockRequestDesc
{
  using timestamp = sma::chrono::system_clock::time_point;
  Hash content_name;
  size_t block_index;
  double utility;
  timestamp expire_time;
  sma::Vec2d origin_location;

  BlockRequestDesc (Hash name,
                    size_t index,
                    double util,
                    timestamp expire,
                    sma::Vec2d location)
      : content_name (name)
      , block_index (index)
      , utility (util)
      , expire_time (expire)
      , origin_location (location)
  {}
};

class comp_blockrequest_desc_utility
{
public:
  bool operator () (BlockRequestDesc & desc1, BlockRequestDesc & desc2)
  {
    if (desc1.utility < desc2.utility)  
      return true;
    else  
      return false;
  }
};
}
