#include <sma/messagetype.hpp>
#include <sma/util/typecodesieve.hpp>

#include <sma/neighbormessage.hpp>
#include <sma/ccn/interestmessage.hpp>
#include <sma/ccn/contentinfomessage.hpp>

namespace sma
{
  using MessageTypes =
    TypecodeSieve<MessageType>
    ::map<0, NeighborMessage>
    ::map<1, ContentInfoMessage>;
}
