#include <sma/ccn/contenthelperimpl.hpp>

#include <sma/context.hpp>
#include <sma/ccn/ccnnode.hpp>

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <sma/messageheader.hpp>
#include <sma/ccn/contentannouncement.hpp>

#include <sma/ccn/interesthelper.hpp>

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

void ContentHelperImpl::receive(MessageHeader header, ContentAnnouncement msg)
{
  bool forward = false;

  auto& descriptor = msg.descriptor;
  // Break loops
  if (descriptor.publisher == node->id)
    return;

  ++descriptor.distance;

  if (node->interests->interested_in(descriptor)) {
    log.d("** I got content I wanted! **");

    auto it = g_pending_content.find(descriptor.type);
    if (it != g_pending_content.end()) {
      auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(
          clock::now() - it->second);
      log.i("** delay: %v ms", delay.count());
      g_pending_content.erase(it);
    }
  }

  if (!update_kct(descriptor))
    return;

  log.d("Content descriptor from n(%v)", header.sender);
  log.d("| distance: %v hops", std::uint32_t(descriptor.distance));
  log.d("| type: %v", descriptor.type);
  log.d("| name: %v", descriptor.name);
  log.d("| publisher: %v", descriptor.publisher);

  if (node->interests->know_remote(descriptor.type)) {
    // log.d("--> (forward)");
    node->post(msg);
  }
  log.d("");
}

bool ContentHelperImpl::update_kct(ContentDescriptor const& descriptor)
{
  auto try_add = kct.emplace(descriptor.hash, descriptor);
  if (try_add.second)
    return true;
  auto& existing = try_add.first->second;
  return existing.update(descriptor);
}

void ContentHelperImpl::publish(ContentType type,
                                ContentName name,
                                std::istream& is)
{
  log.d("Publish content:");
  log.d("| type: %v", type);
  log.d("| name: %v", name);
  log.d("");

  g_pending_content.emplace(type, clock::now());
  auto hash = Hasher(std::string(type))(std::string(name)).digest();
  node->post(ContentAnnouncement(ContentDescriptor(std::move(hash), type, name, node->id)));
}
}
