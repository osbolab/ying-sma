#include <sma/ccn/remotecontent.hpp>

#include <sma/ccn/contenthelper.hpp>


namespace sma
{
RemoteContent::RemoteContent(ContentHelper& helper,
                             ContentMetadata metadata,
                             NetworkDistance distance)
  : helper(helper)
  , meta(metadata)
  , dist(distance)
{
}

bool RemoteContent::saw(NetworkDistance distance)
{
  touch();
  if (distance >= dist)
    return false;

  dist = distance;
  return true;
}
}
