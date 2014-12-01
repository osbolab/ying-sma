#include <sma/ccn/remoteinterest.hpp>
#include <sma/nodeid.hpp>

#include <sma/chrono.hpp>

namespace sma
{
namespace detail
{
  struct RemoteInterestEntry {
    using clock = sma::chrono::system_clock;
    using time_point = clock::time_point;
    using hop_count = RemoteInterest::hop_count;

    hop_count hops;
    time_point last_seen;


    RemoteInterestEntry(RemoteInterest const& ri)
      : hops(ri.hops)
    {
      touch();
    }

    void touch() { last_seen = clock::now(); }

    bool update(RemoteInterest const& ri)
    {
      touch();
      if (ri.hops < hops) {
        hops = ri.hops;
        return true;
      }
      return false;
    }

    template <typename Duration = std::chrono::milliseconds>
    Duration age()
    {
      return std::chrono::duration_cast<Duration>(clock::now() - last_seen);
    }

    template <typename Duration>
    bool older_than(Duration age)
    {
      return this->age<Duration>() >= age;
    }
  };
}
}
