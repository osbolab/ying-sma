#include <sma/ccn/interesthelper.hpp>
#include <sma/ccn/interestmessage.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/context.hpp>

#include <sma/async.hpp>

#include <limits>

namespace sma
{
InterestHelper::InterestHelper(CcnNode* node)
  : node(node)
  , log(node->context->log)
{
}

void InterestHelper::receive(MessageHeader header, InterestMessage msg)
{
  // Don't forward interests we originally sent
  if (msg.interested == node->id)
    return;

  log.t("<-- %v interests from n(%v) via n(%v)",
        msg.interests.size(),
        msg.interested,
        header.sender);

  // Just cull those not to be forwarded from the given vector
  auto it = msg.interests.begin();
  while (it != msg.interests.end()) {
    auto& ri = *it;
    // Account for the link that the message came over;
    // we could increment at the sender, but I prefer counting after the hop.
    ++ri.hops;
    auto result = r_table.emplace(ri.type, detail::RemoteInterestEntry(ri));
    // If we insert a new interest or find a closer node than what we have
    // then we should forward this
    bool updated = result.second ? true : result.first->second.update(ri);
    it = updated ? it + 1 : msg.interests.erase(it);
  }

  if (!msg.interests.empty()) {
    log.t("--> forward %v interests", msg.interests.size());
    node->post(msg);

    log.d("remote interest table (%v):", r_table.size());
    for (auto it = r_table.begin(); it != r_table.end(); ++it)
      log.d("|  %v (%v hops) - last seen: %v ms ago",
            std::string(it->first),
            std::uint32_t(it->second.hops),
            it->second.template age<std::chrono::milliseconds>().count());
  }
}

void InterestHelper::insert_new(std::vector<ContentType> types)
{
  for (auto& t : types)
    table.emplace(t, Interest());

  if (!table.empty())
    schedule_broadcast();
}

void InterestHelper::schedule_broadcast()
{
  auto delay = std::chrono::milliseconds(5000);
  using unit = std::chrono::milliseconds::rep;
  unit min = delay.count() / 2;
  delay = std::chrono::milliseconds(min + rand() % delay.count());
  log.d("broadcast interests in %v ms", delay.count());
  asynctask(std::bind(&InterestHelper::broadcast_interests, this)).do_in(delay);
  return;
}

void InterestHelper::broadcast_interests()
{
  InterestMessage msg(node->id);

  // Our interests are 0 hops from us... the receiver will account for our
  // link to them.
  if (!table.empty()) {
    msg.interests.reserve(table.size());
    for (auto& entry : table)
      msg.interests.emplace_back(entry.first, 0);
  }
  // Add as many remote interests to our message as we can.
  // FIXME: We need some real logic here.
  std::size_t const nmax = 255;
  if (table.size() < nmax) {
    auto nfwds = nmax - table.size();
    for (auto it = r_table.begin(); nfwds-- > 0 && it != r_table.end(); ++it)
      msg.interests.emplace_back(it->first, it->second.hops);
  }

  if (!msg.interests.empty()) {
    log.d("--> %v interests", msg.interests.size());
    node->post(msg);

    schedule_broadcast();
  }
}
}
