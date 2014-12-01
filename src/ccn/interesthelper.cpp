#include <sma/ccn/interesthelper.hpp>
#include <sma/ccn/interestmessage.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/ccn/contentinfo.hpp>

#include <sma/context.hpp>
#include <sma/async.hpp>

#include <chrono>
#include <limits>

using namespace std::literals::chrono_literals;

namespace sma
{
InterestHelper::InterestHelper(CcnNode& node)
  : node(&node)
  , log(node.context->log)
{
}

void InterestHelper::receive(MessageHeader&& header, InterestMessage&& msg)
{
  // Don't forward interests we originally sent
  if (msg.interested_node == node->id)
    return;

  log.t("<-- %v interests from n(%v) via n(%v)",
        msg.interests.size(),
        msg.interested_node,
        header.sender);

  // Just cull those not to be forwarded from the given vector
  for (auto it = msg.interests.begin(); it != msg.interests.end();) {
    auto& interest = *it;
    // Account for the link that the message came over
    ++interest.hops;
    if (!learn_remote_interest(interest))
      // Only forward interests we learned something new about
      it = msg.interests.erase(it);
    else
      ++it;
  }

  if (!msg.interests.empty()) {
    log.t("--> forward %v interests", msg.interests.size());
    node->post(msg);

    log_interest_table();
    schedule_announcement();
  }
}

bool InterestHelper::learn_remote_interest(Interest const& interest)
{
  auto maybe_added = r_table.emplace(interest.type, RemoteInterest(interest));
  if (maybe_added.second)
    return true;
  auto& existing = maybe_added.first->second;
  return existing.update(interest);
}

bool InterestHelper::interested_in(ContentInfo const& info) const
{
  return table.find(info.type) != table.end();
}

bool InterestHelper::know_remote(ContentType const& type) const
{
  return r_table.find(type) != r_table.end();
}

void InterestHelper::insert_new(std::vector<ContentType> types)
{
  for (auto& t : types)
    table.emplace(t, InterestRank());

  if (!table.empty())
    schedule_announcement(200ms);
}

void InterestHelper::schedule_announcement(std::chrono::milliseconds delay)
{
  using unit = std::chrono::milliseconds::rep;
  unit min = delay.count() / 2;
  delay = std::chrono::milliseconds(min + rand() % delay.count());
  log.t("announce interests in %v ms", delay.count());
  asynctask(std::bind(&InterestHelper::announce, this)).do_in(delay);
  return;
}

void InterestHelper::announce()
{
  InterestMessage msg(node->id);

  // Our interests are 0 hops from us... the receiver will account for our
  // link to them.
  if (!table.empty()) {
    msg.interests.reserve(table.size());
    for (auto& entry : table)
      msg.interests.emplace_back(entry.first);
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
    log.t("--> announce %v interests", msg.interests.size());
    node->post(msg);

    schedule_announcement();
  }
}

void InterestHelper::log_interest_table()
{
  log.d("remote interest table (%v):", r_table.size());
  for (auto it = r_table.begin(); it != r_table.end(); ++it)
    log.d("|  %v (%v hops) - last seen: %v ms ago",
          std::string(it->first),
          std::uint32_t(it->second.hops),
          it->second.template age<std::chrono::milliseconds>().count());
}
}
