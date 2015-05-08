#pragma once

#include <sma/ccn/behaviorhelper.hpp>
#include <sma/smafwd.hpp>
#include <sma/util/event.hpp>
#include <sma/io/log>
#include <chrono>
#include <string>
#include <sma/ccn/ccnnode.hpp>
#include <vector>
#include <sma/util/hash.hpp>
#include <unordered_map>


namespace sma
{

class CcnNode;

class BehaviorHelperImpl : public BehaviorHelper
{
  using timestamp = sma::chrono::system_clock::time_point;
  struct Req_Record{
    bool accessed;
    float utility;
    timestamp requested_time;
    timestamp deadline;
    int hops;
    NodeId from_id;
    Req_Record(bool acc, float util, timestamp r_time, timestamp dl, int hops, NodeId id)
      : accessed(acc)
      , utility(util)
      , requested_time(r_time)
      , deadline(dl)
      , hops(hops)
      , from_id(id) {}
  };

public:

  BehaviorHelperImpl (CcnNode& node,
          std::chrono::milliseconds post_interest_freq,
          std::chrono::milliseconds post_contnet_freq,
          std::chrono::milliseconds download_content_freq);

  void behave ();
  bool on_content (Hash hash);

private:
  static const char * alphanum;

  static const std::vector<std::string> categories;

  static constexpr std::size_t max_requests = 1000;
  std::size_t request_times;

  std::chrono::milliseconds interest_freq;
  std::chrono::milliseconds publish_freq;
  std::chrono::milliseconds request_freq;

  std::unordered_map<Hash, Req_Record> content_req_record;

  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;
  using millis = std::chrono::milliseconds;

  void behave_interest ();
  void behave_publish ();
  void behave_request ();
  std::string get_rand_str_n (std::size_t length);

};

}
