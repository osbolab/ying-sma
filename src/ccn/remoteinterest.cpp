#include <sma/ccn/remoteinterest.hpp>
#include <sma/ccn/interest.hpp>

namespace sma
{
RemoteInterest::RemoteInterest(Interest const& interest)
  : distance(interest.distance)
{
  touch();
}

void RemoteInterest::touch() { last_seen = clock::now(); }

bool RemoteInterest::update(Interest const& interest)
{
  touch();
  if (interest.distance < distance) {
    distance = interest.distance;
    return true;
  }
  return false;
}
}
