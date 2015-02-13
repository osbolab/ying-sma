#pragma once

#include <sma/ccn/behaviorhelper.hpp>
#include <sma/smafwd.hpp>
#include <sma/util/event.hpp>
#include <sma/io/log>
#include <chrono>
#include <string>
#include <sma/ccn/ccnnode.hpp>


namespace sma
{

class CcnNode;

class BehaviorHelperImpl : public BehaviorHelper
{
public:

  BehaviorHelperImpl (CcnNode& node,
          std::chrono::milliseconds post_interest_freq,
          std::chrono::milliseconds post_contnet_freq,
          std::chrono::milliseconds download_content_freq);

  void behave ();

private:
  static const char * alphanum;

  std::chrono::milliseconds interest_freq;
  std::chrono::milliseconds publish_freq;
  std::chrono::milliseconds request_freq;

  void behave_interest ();
  void behave_publish ();
  void behave_request ();
  std::string get_rand_str_n (std::size_t length);

};

}
