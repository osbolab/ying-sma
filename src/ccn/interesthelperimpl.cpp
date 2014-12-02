#include <sma/ccn/interesthelperimpl.hpp>
#include <sma/ccn/interestannouncement.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/ccn/contentdescriptor.hpp>

#include <sma/context.hpp>
#include <sma/async.hpp>

#include <chrono>
#include <limits>
#include <sstream>
#include <iomanip>

using namespace std::literals::chrono_literals;

namespace sma
{
void log_interest_table(Logger& log,
                        std::unordered_map<ContentType, RemoteInterest>& rit)
{
  log.d(" remote interest table");
  log.d("| content | age (ms) |");
  log.d("| ------- | -------- |");
  std::stringstream ss;
  for (auto it = rit.begin(); it != rit.end(); ++it) {
    ss.str("");
    auto age_ms = it->second.template age<std::chrono::milliseconds>().count();
    ss << "| " << std::left << std::setw(7) << std::string(it->first) << " | ";
    if (age_ms != 0)
      ss << std::left << std::setw(8) << std::to_string(age_ms) << " |";
    else
      ss << std::left << std::setw(8) << "   *"
         << " |";
    log.d(ss.str());
  }
  log.d("");
}

void InterestHelperImpl::receive(MessageHeader header, InterestAnnouncement msg)
{
  // Ignore loopback
  if (msg.interested_node == node->id)
    return;

  log.t("<-- %v interests from n(%v) via n(%v)",
        msg.interests.size(),
        msg.interested_node,
        header.sender);

  // Cull any that we aren't already broadcasting with a closer distance.
  // The result is that our neighbors always see the shortest path we know of.
  for (auto it = msg.interests.begin(); it != msg.interests.end();) {
    auto& interest = *it;
    if (!learn_remote_interest(interest))
      // Only forward interests we learned something new about
      it = msg.interests.erase(it);
    else
      ++it;
  }

  if (!msg.interests.empty()) {
    log.t("--> forward %v interests", msg.interests.size());
    node->post(msg);

    log_interest_table(log, rit);
    schedule_announcement();
  }
}

bool InterestHelperImpl::learn_remote_interest(Interest const& interest)
{
  auto try_add = rit.emplace(interest.type, RemoteInterest(interest));
  if (try_add.second)
    return true;
  auto& existing = try_add.first->second;
  return existing.update(interest);
}

bool InterestHelperImpl::interested_in(ContentDescriptor const& info) const
{
  return lit.find(info.type) != lit.end();
}

bool InterestHelperImpl::know_remote(ContentType const& type) const
{
  return rit.find(type) != rit.end();
}

void InterestHelperImpl::insert_new(std::vector<ContentType> types)
{
  for (auto& t : types)
    lit.emplace(t, InterestRank());

  if (!lit.empty())
    schedule_announcement(200ms);
}

void InterestHelperImpl::schedule_announcement(std::chrono::milliseconds delay)
{
  using unit = std::chrono::milliseconds::rep;
  unit min = delay.count() / 2;
  delay = std::chrono::milliseconds(min + rand() % delay.count());
  log.t("announce interests in %v ms", delay.count());
  asynctask(std::bind(&InterestHelperImpl::announce, this)).do_in(delay);
  return;
}

void InterestHelperImpl::announce()
{
  InterestAnnouncement msg(node->id);

  // Our interests are 0 hops from us... the receiver will account for our
  // link to them.
  if (!lit.empty()) {
    msg.interests.reserve(lit.size());
    for (auto& entry : lit)
      msg.interests.emplace_back(entry.first);
  }
  // Add as many remote interests to our message as we can.
  // FIXME: We need some real logic here.
  std::size_t const nmax = 255;
  if (lit.size() < nmax) {
    auto nfwds = nmax - lit.size();
    for (auto it = rit.begin(); nfwds-- > 0 && it != rit.end(); ++it)
      msg.interests.emplace_back(it->first);
  }

  if (!msg.interests.empty()) {
    log.t("--> announce %v interests", msg.interests.size());
    node->post(msg);

    schedule_announcement();
  }
}
}
