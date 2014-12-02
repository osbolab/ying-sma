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

#include <unordered_map>

using namespace std::literals::chrono_literals;

namespace sma
{
static std::unordered_map<ContentType, sma::chrono::system_clock::time_point>
    g_pending_content;

ContentHelper::ContentHelper(CcnNode& node)
  : node(&node)
  , log(node.context->log)
{
}

void ContentHelper::receive(MessageHeader header, ContentInfoMessage msg)
{
  bool forward = false;

  auto& info = msg.info;
  // Break loops
  if (info.publisher == node->id)
    return;

  ++info.distance;

  auto& interests = node->interests();
  if (interests.interested_in(info)) {
    log.d("** I got content I wanted! **");

    auto it = g_pending_content.find(info.type);
    if (it != g_pending_content.end()) {
      auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(
          clock::now() - it->second);
      log.i("** delay: %v ms", delay.count());
      g_pending_content.erase(it);
    }
  }

  if (!update_kct(msg.hash, info))
    return;

  log.d("Content info from n(%v)", header.sender);
  log.d("| distance: %v hops", std::uint32_t(info.distance));
  log.d("| type: %v", info.type);
  log.d("| name: %v", info.name);
  log.d("| publisher: %v", info.publisher);

  if (interests.know_remote(info.type)) {
    //log.d("--> (forward)");
    node->post(msg);
  }
  log.d("");
}

bool ContentHelper::update_kct(Hash const& hash, ContentInfo const& info)
{
  auto try_add = kct.emplace(hash, info);
  if (try_add.second)
    return true;
  auto& existing = try_add.first->second;
  return existing.update(info);
}

void ContentHelper::publish(ContentType type,
                            ContentName name,
                            std::istream& is)
{
  log.d("Publish content:");
  log.d("| type: %v", type);
  log.d("| name: %v", name);
  log.d("");

  g_pending_content.emplace(type, clock::now());
  auto hash = Hasher(std::string(type))(std::string(name)).digest();
  node->post(
      ContentInfoMessage(std::move(hash), ContentInfo(type, name, node->id)));
}
}
