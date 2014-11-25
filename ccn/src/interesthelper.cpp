#include <sma/ccn/interesthelper.hpp>

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/interest.hpp>
#include <sma/ccn/remoteinterest.hpp>
#include <sma/ccn/interestmessage.hpp>

namespace sma
{
InterestHelper::InterestHelper(Logger log)
  : log(std::move(log))
{
}

void InterestHelper::receive(InterestMessage const& msg)
{
  log.i("%v interests from %v:", msg.interests.size(), msg.sender);
  for (auto& i : msg.interests) {
    log.i("  %v hops:", i.hops);
    for (auto& t : i.types)
      log.i("  | %v", std::string(t));
  }
}
}
