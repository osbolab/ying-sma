#include <sma/ccn/remoteinterest.hpp>
#include <sma/ccn/interest.hpp>

namespace sma
{
RemoteInterest::RemoteInterest(Interest const& interest)
  : hops(interest.hops)
{
  touch();
}

void RemoteInterest::touch() { last_seen = clock::now(); }

bool RemoteInterest::update(Interest const& interest)
{
  touch();
  if (interest.hops < hops) {
    hops = interest.hops;
    return true;
  }
  return false;
}
}
