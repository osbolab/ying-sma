#include <sma/util/typecode.hpp>
#include <sma/util/typesieve.hpp>

#include <sma/beacon.hpp>
#include <sma/ccn/interestann.hpp>
#include <sma/ccn/contentann.hpp>

namespace sma
{
  using MessageTypecode = Typecode;

  using MessageTypes = TypeSieve<MessageTypecode>
    ::map<0, Beacon>
    ::map<16, InterestAnn>
    ::map<17, ContentAnn>;
}
