#include <sma/util/typecode.hpp>
#include <sma/util/typesieve.hpp>

#include <sma/neighbormessage.hpp>
#include <sma/ccn/interestmessage.hpp>
#include <sma/ccn/contentinfomessage.hpp>

namespace sma
{
  typedef MessageTypecode Typecode;

  using MessageTypes =
    TypeSieve<MessageTypecode>
    ::map<0, NeighborMessage>
    ::map<1, ContentInfoMessage>;
}
