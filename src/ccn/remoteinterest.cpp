#include <sma/ccn/remoteinterest.hpp>
#include <sma/ccn/interest.hpp>

namespace sma
{
RemoteInterest::RemoteInterest(std::uint32_t hops)
  : hops(hops)
{
  touch();
}

void RemoteInterest::touch() { last_seen = clock::now(); }

bool RemoteInterest::update(std::uint32_t hops)
{
  touch();
  if (hops < this->hops) {
    this->hops = hops;
    return true;
  }
  return false;
}
}
