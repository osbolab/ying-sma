#include <sma/ccn/contenthelper.hpp>

#include <sma/context.hpp>
#include <sma/ccn/ccnnode.hpp>

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <sma/messageheader.hpp>
#include <sma/ccn/contentinfomessage.hpp>

#include <sma/chrono.hpp>

#include <ctime>
#include <chrono>
#include <istream>

using namespace std::literals::chrono_literals;

namespace sma
{
ContentHelper::ContentHelper(CcnNode& node)
  : node(&node)
  , log(node.context->log)
{
}

void ContentHelper::receive(MessageHeader header, ContentInfoMessage msg)
{
  using clock = sma::chrono::system_clock;

  ++msg.hops;

  auto& info = msg.info;
  log.d("Content info from %v", header.sender);
  log.d("| distance: %v hops", msg.hops);
  log.d("| type: %v", info.type);
  log.d("| name: %v", info.name);
  log.d("| creator: %v", info.originating_node);
  log.d("| publisher: %v", info.publishing_node);
  log.d("| created at: %v",
        clock::utcstrftime(clock::from_time_t(info.creation_time)));

  auto& interests = node->interests();
  if (interests.interested_in(info))
    log.d("I got content I wanted!");

  if (interests.know_remote(info.type)) {
    bool forward = true;
    auto expiration = clock::now() + 10s;
    auto maybe_record = forward_record.emplace(info, expiration);

    if (!maybe_record.second) {
      auto& existing = maybe_record.first;
      if (existing->second - clock::now() <= 0s)
        existing->second = expiration;
      else
        forward = false;
    }

    if (forward) {
      log.d("--> I know someone who wants this");
      node->post(msg);
    }
  }
}

void ContentHelper::publish(ContentType type,
                            ContentName name,
                            std::istream& is)
{
  log.d("Publish content:");
  log.d("| type: %v", type);
  log.d("| name: %v", name);

  node->post(ContentInfoMessage(ContentInfo(type, name, node->id)));
}
}
