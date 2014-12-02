#include <sma/util/typecode.hpp>
#include <sma/util/typesieve.hpp>

#include <sma/beacon.hpp>
#include <sma/ccn/interestannouncement.hpp>
#include <sma/ccn/contentannouncement.hpp>

namespace sma
{
  using MessageTypecode = Typecode;

  using MessageTypes = TypeSieve<MessageTypecode>
    ::map<0, Beacon>
    ::map<1, InterestAnnouncement>
    ::map<2, ContentAnnouncement>;
}
