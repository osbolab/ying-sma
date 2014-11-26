#include <sma/ccn/interesthelper.hpp>

#include <sma/ccn/ccnnode.hpp>
#include <sma/message.hpp>
#include <sma/ccn/interestmessage.hpp>

#include <limits>

namespace sma
{
InterestHelper::InterestHelper(CcnNode* node)
  : node(node)
  , log(node->context()->log())
{
}

void InterestHelper::receive(InterestMessage msg)
{
  // Just cull those not to be forwarded from the given vector
  auto will_forward = std::move(msg.interests);

  auto it = will_forward.begin();
  while (it != will_forward.end()) {
    auto& ri = *it;
    // Account for the link that the message came over;
    // we could increment at the sender, but I prefer counting after the hop.
    ++ri.hops;
    auto result = r_table.emplace(ri.type, detail::RemoteInterestEntry(ri));
    // If we insert a new interest or find a closer node than what we have
    // then we should forward this
    bool updated = result.second ? true : result.first->second.update(ri);
    it = updated ? it + 1 : will_forward.erase(it);
  }

  if (!will_forward.empty()) {
    log.d("forwarding %v interests", will_forward.size());
    node->post(
        InterestMessage(node->id(), std::move(will_forward)).make_message());
  }

  log.i("remote interests (%v):", r_table.size());
  for (auto it = r_table.begin(); it != r_table.end(); ++it)
    log.i("  %v (%v hops) - last seen: %v s ago",
          std::string(it->first),
          std::uint32_t(it->second.hops),
          it->second.template age<std::chrono::seconds>().count());
}

void InterestHelper::add(std::vector<ContentType> types)
{
  for (auto& t : types)
    table.emplace(t, Interest());

  if (!table.empty())
    broadcast_interests();
}

void InterestHelper::broadcast_interests(bool schedule_only)
{
  if (schedule_only) {
    auto delay = std::chrono::milliseconds(100);
    using unit = std::chrono::milliseconds::rep;
    unit min = delay.count() / 2;
    delay = std::chrono::milliseconds(min + rand() % delay.count());
    node->async(std::bind(&InterestHelper::broadcast_interests, this, false))
        .do_in(delay);
    return;
  }

  InterestMessage msg(node->id());

  // Our interests are 0 hops from us... the receive will account for our
  // link to them.
  if (!table.empty()) {
    msg.interests.reserve(table.size());
    for (auto& entry : table)
      msg.interests.emplace_back(entry.first, 0);
  }
  // Add as many remote interests to our message as we can.
  // FIXME: We need some real logic here.
  auto const nmax = std::numeric_limits<InterestMessage::count_type>::max();
  if (table.size() < nmax) {
    InterestMessage::count_type nfwds = nmax - table.size();
    auto it = r_table.begin();
    while (nfwds-- > 0 && it != r_table.end())
      msg.interests.emplace_back(it->first, it->second.hops);
  }

  if (!msg.interests.empty())
    node->post(msg.make_message());
}
}
