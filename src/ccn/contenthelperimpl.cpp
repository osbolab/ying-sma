#include <sma/ccn/contenthelperimpl.hpp>

#include <sma/context.hpp>
#include <sma/ccn/ccnnode.hpp>

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <sma/messageheader.hpp>
#include <sma/ccn/contentann.hpp>

#include <sma/ccn/interesthelper.hpp>

#include <sma/chrono.hpp>

#include <ctime>
#include <chrono>
#include <istream>

using namespace std::literals::chrono_literals;

namespace sma
{
static sma::chrono::system_clock::time_point g_published;

void ContentHelperImpl::receive(MessageHeader header, ContentAnn msg)
{
  bool forward = false;

  auto& descriptor = msg.descriptor;
  // Break loops
  if (descriptor.publisher == node->id)
    return;

  ++descriptor.distance;

  if (node->interests->interested_in(descriptor)) {
    log.d("** I got content I wanted! **");
  }

  if (!update_kct(descriptor))
    return;

  /*
  log.d("Content descriptor from n(%v)", header.sender);
  log.d("| distance: %v hops", std::uint32_t(descriptor.distance));
  log.d("| type: %v", descriptor.type);
  log.d("| name: %v", descriptor.name);
  log.d("| publisher: %v", descriptor.publisher);
  */
  log.d("got it");

    log.d(descriptor.name.size());
  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
      clock::now() - g_published);
  log.i("delay: %v ms", time.count());

  if (node->interests->know_remote(descriptor.type)) {
    log.d("--> (forward)");
    node->post(msg);
  }
  else
    log.d("not forwarding");
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
  g_published = clock::now();

  log.d("Publish content:");
  // log.d("| type: %v", type);
  // log.d("| name: %v", name);
  log.d("");

  auto hash = Hasher(std::string(type))(std::string(name)).digest();
  node->post(ContentAnn(
      ContentDescriptor(std::move(hash), type, name, node->id)));
}
}
